#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/Practical.h"

int main(int Argc, char* Argv[])
{
    if (Argc < 3 || Argc > 4)
    {
        DieWithUserMessage("Parameter(s)", "<Server Address> <Echo Word> [<Server Port>]");
    }

    char* ServerIP = Argv[1];
    char* EchoString = Argv[2];

    in_port_t ServerPort = (Argc == 4) ? atoi(Argv[3]) : 7; // convert port representation from string to 16-bit binary

    /**
     * AF_INET: IPv4 Internet protocols
     * SOCK_STREAM: Provides sequenced, reliable, two-way, connection-based byte streams. An out-of-band data transmission mechanism may be supported.
     */
    int ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket < 0)
    {
        DieWithSystemMessage("socket() failed");
    }

    struct sockaddr_in ServerAddress; // information container for address and port to connect to
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET; // address family

    int ReturnValue = inet_pton(AF_INET, ServerIP, &ServerAddress.sin_addr.s_addr); // convert IP representation from string to 32-bit binary
    if (ReturnValue == 0)
    {
        DieWithUserMessage("inet_pton() failed", "invalid address string");
    }
    else if (ReturnValue < 0)
    {
        DieWithSystemMessage("inet_pton() failed");
    }
    ServerAddress.sin_port = htons(ServerPort); // convert IP port number in host byte order to network byte order

    if (connect(ClientSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
    {
        DieWithSystemMessage("connect() failed");
    }

    size_t EchoStringLength = strlen(EchoString);

    ssize_t NumBytes = send(ClientSocket, EchoString, EchoStringLength, 0);
    if (NumBytes < 0)
    {
        DieWithSystemMessage("send() failed");
    }
    else if (NumBytes != EchoStringLength)
    {
        DieWithUserMessage("send()", "sent unexpected number of bytes");
    }

    unsigned int TotalBytesReceived = 0;
    fputs("Received: ", stdout);
    /**
     * The bytes sent by a call to send() on one end of a connection may not all be returned by a single call to recv() on the other end
     * So we need to repeatedly receive bytes until we have received as many as we sent
     */
    while (TotalBytesReceived < EchoStringLength)
    {
        char Buffer[BUFSIZ];
        
        NumBytes = recv(ClientSocket, Buffer, BUFSIZ - 1, 0);
        if (NumBytes < 0)
        {
            DieWithSystemMessage("recv() failed");
        }
        else if (NumBytes == 0)
        {
            DieWithUserMessage("recv()", "connection closed prematurely");
        }

        TotalBytesReceived += NumBytes;
        Buffer[NumBytes] = '\0';
        fputs(Buffer, stdout);
    }

    fputc('\n', stdout);

    close(ClientSocket);
    exit(0);
}