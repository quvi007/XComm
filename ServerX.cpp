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
#include <vector>
#include <pthread.h>
#include <set>

#define BACKLOG 127

using namespace std;

int k;

struct args {
    int *k;
    int *clients;
    string *clientNames;
    int src;
};

char *serverName;
char *port;

set<int> excludedList;

void sendToClients (int *k, int *clients, string *clientNames, int src, const char *msg, int flag = 0) {
    if (excludedList.count(src)) return;
    for (int i = 0; i < *k; ++i) {
        if (excludedList.count(i)) continue;
        if (i == src) continue;
        string txtToSend = string(msg, 0, strlen(msg));
        if (flag == 0) txtToSend = clientNames[src] + ": " + txtToSend;
        send(clients[i], txtToSend.c_str(), txtToSend.length() + 1, 0);
    }
    if (flag == 1) {
        string greet = "Welcome to Server \"" + string(serverName, 0, strlen(serverName)) + "\", " + clientNames[src];
        send(clients[src], greet.c_str(), greet.length() + 1, 0);
    }
}

void *ReadFromClient(void *p) {
    args *myArgs = ((args *) p);
    int *k = myArgs->k;
    int *clients = myArgs->clients;
    string *clientNames = myArgs->clientNames;
    int src = myArgs->src;

    if (excludedList.count(src) == 0) {
        char buff[256]{0};

        while (1) {
            memset(buff, 0, sizeof buff);
            int rbSize = recv(clients[src], buff, sizeof buff, 0);
            if (rbSize <= 0) {
                string prompt = "\"" + clientNames[src] + "\" disconnected from the server";
                cout << prompt << "\n";
                sendToClients(k, clients, clientNames, src, prompt.c_str(), 2);
                excludedList.insert(src);
                close(clients[src]);
                break;
            }
            sendToClients(k, clients, clientNames, src, buff);
        }
    }
    pthread_exit(0);
}

int main(int argc, char **argv) {
    serverName = argv[1];
    port = argv[2];
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);

    listen(sockfd, BACKLOG);

    int newfds[BACKLOG]{0};
    sockaddr_in their_addresses[BACKLOG];
    socklen_t their_sizes[BACKLOG];

    pthread_t tids_recv[BACKLOG];
    pthread_attr_t attrs_recv[BACKLOG];
    string clientNames[BACKLOG];

    for (int i = 0; i < BACKLOG; ++i) {
        pthread_attr_init(&attrs_recv[i]);
    }

    for (int i = 0; i < BACKLOG; ++i) {
        their_sizes[i] = sizeof their_addresses[i];
        newfds[i] = accept(sockfd, (sockaddr *) &their_addresses[i], &their_sizes[i]);
        char tBuff[256]{0};
        int rbSizeF = recv(newfds[i], tBuff, sizeof tBuff, 0);
        clientNames[i] = string(tBuff, 0, rbSizeF);
        char theirIpBuff[INET_ADDRSTRLEN]{0};
        string clientIp;
        inet_ntop(AF_INET, &(their_addresses[i].sin_addr.s_addr), theirIpBuff, sizeof theirIpBuff);
        clientIp = string(theirIpBuff, 0, strlen(theirIpBuff));
        int theirPort = ntohs(their_addresses[i].sin_port);
        string prompt = "\"" + clientNames[i] + "\" connected to the server from IP " + clientIp + ":" + to_string(theirPort);
        cout << prompt << "\n";
        k++;
        sendToClients(&k, newfds, clientNames, i, prompt.c_str(), 1);
        args myArgs;
        myArgs.clientNames = clientNames;
        myArgs.clients = newfds;
        myArgs.k = &k;
        myArgs.src = i;

        pthread_create(&tids_recv[i], &attrs_recv[i], ReadFromClient, &myArgs);
    }

    close(sockfd);
    for (int i = 0; i < k; ++i) {
        pthread_join(tids_recv[i], NULL);
    }

    for (int i = 0; i < k; ++i) {
        close(newfds[i]);
    }


    return 0;
}
