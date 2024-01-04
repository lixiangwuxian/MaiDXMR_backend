import socket

# 创建一个UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# 绑定socket到端口
server_address = ('', 10890)
sock.bind(server_address)

print("Listening on port 10890 for UDP messages...")

# 监听并打印接收到的消息
while True:
    data, address = sock.recvfrom(4096)
    print(f"Received message: {data} from {address}")
