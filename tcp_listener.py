import socket

def start_server(host='0.0.0.0', port=10890):
    # 创建 socket 对象
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        # 绑定端口
        server_socket.bind((host, port))
        # 开始监听
        server_socket.listen()

        print(f"Server listening on {host}:{port}...")

        # 等待客户端连接
        conn, addr = server_socket.accept()
        with conn:
            print(f"Connected by {addr}")
            while True:
                # 接收数据
                data = conn.recv(1024)
                if not data:
                    break
                # 打印接收到的数据
                print(f"Received: {data.decode()}")
                # 回复客户端
                conn.sendall(data)

if __name__ == '__main__':
    start_server()