//For Unix Environment

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;

int k;

char *clientName;
char *serverIp;
char *port;
bool stop_flag = false;

void *ReadFromServer(void *p) {
    int sockfd = *((int *) p);
    char buff[256]{0};
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
    pthread_exit(0);
}

void *SendToServer(void *p) {
    int sockfd = *((int *) p);
    while (1) {
        if (stop_flag) break;
        string str;
        getline(cin, str);
        send(sockfd, str.c_str(), str.length() + 1, 0);
    }
    pthread_exit(0);
}

int main(int argc, char **argv) {
    clientName = argv[1];
    serverIp = argv[2];
    port = argv[3];

    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(serverIp, port, &hints, &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sockfd, res->ai_addr, res->ai_addrlen);

    send(sockfd, clientName, strlen(clientName) + 1, 0);

    pthread_t tid_recv, tid_send;
    pthread_attr_t attr_recv, attr_send;

    pthread_attr_init(&attr_recv);
    pthread_attr_init(&attr_send);

    pthread_create(&tid_recv, &attr_recv, ReadFromServer, &sockfd);
    pthread_create(&tid_send, &attr_send, SendToServer, &sockfd);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    close(sockfd);

    return 0;
}
