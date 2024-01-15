#include <string>
#include <vector>
#include <winsock2.h>

// #pragma comment(lib, "Ws2_32.lib")

class UDPClient {
public:
  UDPClient(const std::string &ip, int port) : serverIp(ip), serverPort(port) {
    // 初始化 Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建 socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // 设置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(serverIp.c_str());
  }

  ~UDPClient() {
    closesocket(sock);
    WSACleanup();
  }

  void send(const std::vector<uint8_t> &data) {
    // int dataLength = static_cast<int>(data.size());

    // // 将长度信息转换为字节
    // std::vector<uint8_t> lengthBytes(sizeof(dataLength));
    // for (size_t i = 0; i < sizeof(dataLength); ++i) {
    //   lengthBytes[i] = (dataLength >> (i * 8)) & 0xFF;
    // }

    // // 将长度字节和数据组合到一起
    // std::vector<uint8_t> padding(4, 0);
    // std::vector<uint8_t> sendData;
    // sendData.reserve(lengthBytes.size() + data.size());
    // sendData.insert(sendData.end(), lengthBytes.begin(), lengthBytes.end());
    // sendData.insert(sendData.end(), data.begin(), data.end());
    // sendData.insert(sendData.end(), padding.begin(), padding.end());

    // 发送组合后的数据
    sendto(sock, reinterpret_cast<const char *>(data.data()),
           data.size(), 0, reinterpret_cast<sockaddr *>(&serverAddr),
           sizeof(serverAddr));
  }

private:
  SOCKET sock;
  struct sockaddr_in serverAddr;
  std::string serverIp;
  int serverPort;
};