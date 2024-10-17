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
        DieWithUserMessage("Parameter(s)", "<Server Address/Name> <Echo Word> [<Server Port>]");
    }

    char* Server = Argv[1];
    char* EchoString = Argv[2];
    char* Service = (Argc == 4) ? Argv[3] : "echo";

    int ClientSocket = SetupTCPClientSocket(Server, Service);
    if (ClientSocket < 0)
    {
        DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");
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