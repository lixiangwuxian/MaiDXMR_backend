#!/usr/bin/env python3
"""Stream test video to the MaiDXMR headset app without the Windows backend.

Sends what DesktopCaptureApp sends: one H.264 frame (Annex B access unit) per
UDP datagram to port 10890 on the headset. ffmpeg captures and encodes; this
script forwards ffmpeg's packets to the headset and shows live stats.

Examples:
  python3 stream_test.py 192.168.1.50                  # test pattern with a counter
  python3 stream_test.py 192.168.1.50 --source screen  # stream the desktop

Measuring latency with the test pattern: the headset shows a counter
(elapsed seconds * 100). The status line here shows the counter of the frame
that was just sent. Look at both through passthrough; the difference / 100 is
the delay in seconds.
"""

import argparse
import collections
import os
import platform
import shlex
import socket
import subprocess
import sys
import threading
import time

MAX_DATAGRAM = 65507  # largest UDP payload over IPv4
SOCKET_BUFFER = 4 * 1024 * 1024


def parse_size(text):
    width, height = text.lower().split("x")
    return int(width), int(height)


def parse_crop(text):
    size, x, y = text.split("+")
    return parse_size(size) + (int(x), int(y))


def input_args(args):
    """Return the ffmpeg input arguments and any extra video filters."""
    width, height = args.size
    if args.source == "test":
        # testsrc draws elapsed seconds * 100 as big digits (no decimal point);
        # realtime paces it like a live source.
        src = f"testsrc=size={width}x{height}:rate={args.fps}:decimals=2,realtime"
        return ["-f", "lavfi", "-i", src], []

    system = platform.system()
    fps = str(args.fps)
    filters = []
    if system == "Linux":
        # x11grab sees X11 (and XWayland) windows only.
        display = args.display or os.environ.get("DISPLAY") or ":0"
        inp = ["-f", "x11grab", "-framerate", fps, "-draw_mouse", "0"]
        if args.crop:
            w, h, x, y = args.crop
            inp += ["-video_size", f"{w}x{h}", "-i", f"{display}+{x},{y}"]
        else:
            inp += ["-i", display]
    elif system == "Darwin":
        # Needs the Screen Recording permission for the terminal app.
        inp = ["-f", "avfoundation", "-framerate", fps, "-capture_cursor", "0",
               "-i", f"Capture screen {args.screen}:none"]
        if args.crop:
            filters.append("crop={}:{}:{}:{}".format(*args.crop))
    elif system == "Windows":
        inp = ["-f", "gdigrab", "-framerate", fps, "-draw_mouse", "0"]
        if args.crop:
            w, h, x, y = args.crop
            inp += ["-offset_x", str(x), "-offset_y", str(y),
                    "-video_size", f"{w}x{h}"]
        inp += ["-i", "desktop"]
    else:
        sys.exit(f"Screen capture is not supported on {system}, use --source test")

    # Fit the capture into the stream size without distorting it.
    filters.append(f"scale={width}:{height}:force_original_aspect_ratio=decrease"
                   ":flags=bilinear")
    filters.append(f"pad={width}:{height}:(ow-iw)/2:(oh-ih)/2")
    return inp, filters


def encoder_args(args):
    bitrate = int(args.bitrate * 1_000_000)
    # One-frame VBV like the NVENC backend: no frame gets much bigger than
    # bitrate / fps, so each one fits in a single datagram.
    vbv = int(bitrate / args.fps * 1.1)
    return ["-an", "-c:v", "libx264", "-preset", "ultrafast", "-tune", "zerolatency",
            "-bf", "0", "-g", str(args.gop or args.fps), "-pix_fmt", "yuv420p",
            "-b:v", str(bitrate), "-maxrate", str(bitrate), "-bufsize", str(vbv)]


def set_buffer(sock, option):
    try:
        sock.setsockopt(socket.SOL_SOCKET, option, SOCKET_BUFFER)
    except OSError:
        pass


class Status:
    """A status line that is redrawn in place, with normal lines printed above it."""

    def __init__(self):
        self.lock = threading.Lock()
        self.width = 0

    def line(self, text):
        with self.lock:
            sys.stdout.write("\r" + text.ljust(self.width))
            sys.stdout.flush()
            self.width = len(text)

    def message(self, text):
        with self.lock:
            sys.stdout.write("\r" + " " * self.width + "\r" + text.rstrip() + "\n")
            sys.stdout.flush()
            self.width = 0


def format_status(args, frames, recent, oversize, send_errors):
    if args.source == "test" and frames:
        # The number the test pattern shows in the frame just sent.
        parts = [f"counter {(frames - 1) * 100 // args.fps}"]
    else:
        parts = [f"frame {frames}"]
    total = sum(size for _, size in recent)
    parts.append(f"{len(recent)} fps")
    parts.append(f"{total * 8 / 1e6:.1f} Mbit/s")
    if recent:
        largest = max(size for _, size in recent)
        parts.append(f"avg {total / len(recent) / 1024:.0f} KB max {largest / 1024:.0f} KB")
    if oversize:
        parts.append(f"{oversize} frames too big for one datagram")
    if send_errors:
        parts.append(f"{send_errors} send errors")
    return " | ".join(parts)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("headset_ip", help="IP address of the Quest running MaiDXMR")
    parser.add_argument("--port", type=int, default=10890,
                        help="headset UDP port (default: 10890)")
    parser.add_argument("--source", choices=["test", "screen"], default="test",
                        help="test: generated pattern with a counter (default), "
                             "screen: capture the desktop")
    parser.add_argument("--size", type=parse_size, default=(1080, 1920), metavar="WxH",
                        help="stream resolution (default: 1080x1920)")
    parser.add_argument("--fps", type=int, default=60, help="frame rate (default: 60)")
    parser.add_argument("--bitrate", type=float, default=20,
                        help="bitrate in Mbit/s (default: 20)")
    parser.add_argument("--gop", type=int, default=0,
                        help="keyframe interval in frames (default: one per second)")
    parser.add_argument("--crop", type=parse_crop, metavar="WxH+X+Y",
                        help="capture only this screen region (--source screen)")
    parser.add_argument("--display", help="X11 display to capture (Linux, default: $DISPLAY)")
    parser.add_argument("--screen", type=int, default=0,
                        help="index of the screen to capture (macOS, default: 0)")
    parser.add_argument("--ffmpeg", default="ffmpeg", help="ffmpeg executable")
    args = parser.parse_args()

    if args.bitrate * 1e6 / args.fps * 1.1 / 8 > MAX_DATAGRAM:
        print(f"warning: at {args.bitrate:g} Mbit/s and {args.fps} fps a frame can exceed "
              f"one UDP datagram ({MAX_DATAGRAM} bytes) and will not decode on the "
              "headset; lower --bitrate", file=sys.stderr)

    # ffmpeg sends each encoded frame as one datagram to this local socket.
    rx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    set_buffer(rx, socket.SO_RCVBUF)
    rx.bind(("127.0.0.1", 0))
    rx.settimeout(0.5)
    tx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # macOS rejects datagrams larger than the send buffer (9216 bytes by default).
    set_buffer(tx, socket.SO_SNDBUF)
    dest = (args.headset_ip, args.port)

    inp, filters = input_args(args)
    cmd = [args.ffmpeg, "-hide_banner", "-loglevel", "warning", "-nostdin"] + inp
    if filters:
        cmd += ["-vf", ",".join(filters)]
    cmd += encoder_args(args)
    cmd += ["-flush_packets", "1", "-f", "h264",
            f"udp://127.0.0.1:{rx.getsockname()[1]}"
            f"?pkt_size={MAX_DATAGRAM}&buffer_size={SOCKET_BUFFER}"]
    print(shlex.join(cmd))
    print(f"sending to {dest[0]}:{dest[1]}, Ctrl+C to stop")

    status = Status()
    try:
        ffmpeg = subprocess.Popen(cmd, stdin=subprocess.DEVNULL, stderr=subprocess.PIPE,
                                  text=True, errors="replace")
    except FileNotFoundError:
        sys.exit(f"{args.ffmpeg} not found, install ffmpeg or pass --ffmpeg")

    def relay_ffmpeg_log():
        for log_line in ffmpeg.stderr:
            status.message("[ffmpeg] " + log_line)

    log_thread = threading.Thread(target=relay_ffmpeg_log, daemon=True)
    log_thread.start()

    frames = oversize = send_errors = 0
    recent = collections.deque()  # (time, size) of datagrams in the last second
    next_status = 0.0
    ffmpeg_exited = False
    try:
        while True:
            try:
                data = rx.recv(65535)
            except socket.timeout:
                if ffmpeg.poll() is not None:
                    ffmpeg_exited = True
                    break
                continue
            try:
                tx.sendto(data, dest)
            except OSError as e:
                send_errors += 1
                if send_errors == 1:
                    status.message(f"sending to {dest[0]}:{dest[1]} failed: {e}")
            # A frame too big for one datagram arrives as a full one plus a tail
            # that doesn't start with a start code.
            if data.startswith(b"\0\0\1") or data.startswith(b"\0\0\0\1"):
                frames += 1
            if len(data) >= MAX_DATAGRAM:
                oversize += 1

            now = time.monotonic()
            recent.append((now, len(data)))
            while recent[0][0] < now - 1.0:
                recent.popleft()
            if now >= next_status:
                next_status = now + 0.05
                status.line(format_status(args, frames, recent, oversize, send_errors))
    except KeyboardInterrupt:
        pass
    finally:
        if ffmpeg.poll() is None:
            ffmpeg.terminate()
            try:
                ffmpeg.wait(timeout=3)
            except subprocess.TimeoutExpired:
                ffmpeg.kill()
        log_thread.join(timeout=1)
        status.message(f"sent {frames} frames, {oversize} too big for one datagram, "
                       f"{send_errors} send errors")
    if ffmpeg_exited:
        sys.exit(f"ffmpeg exited with code {ffmpeg.returncode}, see its messages above")


if __name__ == "__main__":
    main()
