Work In Progress...

Backend for maimai dx with hand tracking.

Todo List:

- [X] capture sinmai.exe display
- [X] encode texture to h264 stream
  - [ ] in low latency mode - failed on decoder side
- [ ] encode the game sound to headset
- [X] send the stream to headset in udp
- [ ] full hand hitbox
- [ ] add a user-friendly config file
- [X] receive user input from quest in udp
- [X] transform the input to proper serial data format
- [X] simulate the keyboard input
- [X] send input data to sinmai.exe
- [ ] stop streaming when hmd not active
- [ ] simulate light

## Build & Run Cpp part

```batch
mkdir build
cd build
cmake ..
msbuild DesktopCaptureApp.sln /p:Configuration=Release
```

You can find the output binary in `build/Release/` folder.

PS: If you encounts the error `msbuild` not found, you have to add the path to `msbuild.exe` to your `PATH` environment variable. See `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`, you can add this path to your `PATH` environment variable. Or you can just open the Visual Studio 2022 IDE and build the solution there.

## Cross-compile on Linux / macOS

The program only runs on Windows (it captures the Sinmai window with GDI and encodes with NVENC through D3D11), but the `.exe` can be built without Windows using MinGW-w64:

```bash
# Ubuntu/Debian: sudo apt install mingw-w64 cmake
# macOS:         brew install mingw-w64 cmake
cmake -S . -B build-mingw -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-mingw -j
```

The output `build-mingw/DesktopCaptureApp.exe` is statically linked and only depends on system DLLs, so it can be copied to the Windows PC as is.

## Test the headset app without the backend

`tools/stream_test.py` uses ffmpeg to send the same kind of stream as `DesktopCaptureApp` (one H.264 frame per UDP datagram to port 10890), so the Quest app can be tested from Linux or macOS. It needs Python 3.8+ and ffmpeg with libx264 (`sudo apt install ffmpeg` / `brew install ffmpeg`).

```bash
python3 tools/stream_test.py <quest-ip>                  # test pattern with a counter
python3 tools/stream_test.py <quest-ip> --source screen  # capture the desktop
```

With the test pattern, the status line shows the counter of the frame that was just sent. Compare it with the number shown in the headset (look at the monitor through passthrough): the difference divided by 100 is the latency in seconds. See `--help` for resolution, bitrate, cropping and other options.

Screen capture uses x11grab on Linux (X11/XWayland windows only, so use the test pattern under Wayland), avfoundation on macOS (the terminal needs the Screen Recording permission) and gdigrab on Windows.

## Debug the Cpp part

Open this project with VS Code, and press `F5` to start debugging. Make sure you have already opened the `Sinmai.exe` window before you start debugging, or the program will crash.

## Run the Python part

You need to install the following packages:

```batch
pip install pynput pyserial bitarray
```

Just double click the `pysrc\run_me.bat` file.
