
import socket
from pyserial import SerialManager
from pykey import KeyboardSimulator

def main():
    # 创建一个UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    serialManager = SerialManager()
    keyboardSim=KeyboardSimulator()
    # 绑定socket到端口
    server_address = ('', 10890)
    sock.bind(server_address)

    print("Listening on port 10890 for UDP messages...")
    # 监听并打印接收到的消息
    while True:
        try:
            data, address = sock.recvfrom(4)
            input_type = data[0]
            key_code = data[1]
            is_pressed = data[2] == 1
            print(f"input_type:{input_type},key_code:{key_code},is_pressed:{is_pressed}")
            if input_type == 0:
                serialManager.change_touch(True, key_code, is_pressed)
                serialManager.ping_touch_thread()
            elif input_type == 1:
                keyboardSim.press_or_release_key(key_code, is_pressed)
        except:
            sock.shutdown(0)
            serialManager.shutdown_now()
            return

main()