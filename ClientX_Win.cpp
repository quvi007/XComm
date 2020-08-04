//For Windows Environment

#include <iostream>
#include <sys/types.h>
#include <WS2tcpip.h>

#include <string>
#include <cstring>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int k;

char* clientName;
char* serverIp;
char* port;
bool stop_flag = false;

void ReadFromServer(void* p) {
    int sockfd = *((int*)p);
    char buff[256]{ 0 };
    while (1) {
        memset(buff, 0, sizeof buff);
        int rbSize = recv(sockfd, buff, sizeof buff, 0);
        if (rbSize <= 0) {
            cout << "The Server Disconnected\n";
            stop_flag = true;
            break;
        }
        cout << buff << "\n";
    }
}

void SendToServer(void* p) {
    int sockfd = *((int*)p);
    while (1) {
        if (stop_flag) break;
        string str;
        getline(cin, str);
        send(sockfd, str.c_str(), str.length() + 1, 0);
    }
}

int main(int argc, char** argv) {
    WSAData wsData;
    WSAStartup(MAKEWORD(2, 2), &wsData);

    clientName = argv[1];
    serverIp = argv[2];
    port = argv[3];

    addrinfo hints, * res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(serverIp, port, &hints, &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sockfd, res->ai_addr, res->ai_addrlen);

    send(sockfd, clientName, strlen(clientName) + 1, 0);

    thread t1(ReadFromServer, &sockfd);
    thread t2(SendToServer, &sockfd);

    t1.join();
    t2.join();

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
