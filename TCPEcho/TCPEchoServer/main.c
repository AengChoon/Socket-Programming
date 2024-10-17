#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/Practical.h"

static const int MAXPENDING = 5;

void HandleTCPClient(int ClientSocket)
{
    char Buffer[BUFSIZ];

    ssize_t NumBytesReceived = recv(ClientSocket, Buffer, BUFSIZ, 0);
    if (NumBytesReceived < 0)
    {
        DieWithSystemMessage("recv() failed");
    }

    while (NumBytesReceived > 0)
    {
        ssize_t NumBytesSent = send(ClientSocket, Buffer, NumBytesReceived, 0);
        if (NumBytesSent < 0)
        {
            DieWithSystemMessage("send() failed");
        }
        else if (NumBytesSent != NumBytesReceived)
        {
            DieWithUserMessage("send()", "send unexpected number of bytes");
        }

        NumBytesReceived = recv(ClientSocket, Buffer, BUFSIZ, 0);
        if (NumBytesReceived < 0)
        {
            DieWithSystemMessage("recv() failed");
        }
    }

    close(ClientSocket);
}

int main(int Argc, char* Argv[])
{
    if (Argc != 2)
    {
        DieWithUserMessage("Parameter(s)", "<Server Port>");
    }

    in_port_t ServerPort = atoi(Argv[1]);

    int ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket < 0)
    {
        DieWithSystemMessage("socket() failed");
    }

    struct sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY); // convert IPv4 address in host byte order to network byte order
    ServerAddress.sin_port = htons(ServerPort);

    if (bind(ServerSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
    {
        DieWithSystemMessage("bind() failed");
    }

    if (listen(ServerSocket, MAXPENDING) < 0)
    {
        DieWithSystemMessage("listen() failed");
    }

    for (;;)
    {
        struct sockaddr_in ClientAddress;
        socklen_t ClientAddressLength = sizeof(ClientAddress);

        int ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress, &ClientAddressLength);
        if (ClientSocket < 0)
        {
            DieWithSystemMessage("accept() failed");
        }

        char ClientName[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &ClientAddress.sin_addr.s_addr, ClientName, sizeof(ClientName)) != NULL)
        {
            printf("Handling client %s/%d\n", ClientName, ntohs(ClientAddress.sin_port));
        }
        else
        {
            puts("Unable to get client address");
        }

        HandleTCPClient(ClientSocket);
    }
}