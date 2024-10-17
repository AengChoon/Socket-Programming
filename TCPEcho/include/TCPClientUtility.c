#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"

int SetupTCPClientSocket(const char* Host, const char* Service)
{
    struct addrinfo AddressCriteria;
    memset(&AddressCriteria, 0, sizeof(AddressCriteria));
    AddressCriteria.ai_family = AF_UNSPEC;
    AddressCriteria.ai_socktype = SOCK_STREAM;
    AddressCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo* ServerAddressInfos;
    int ReturnValue = getaddrinfo(Host, Service, &AddressCriteria, &ServerAddressInfos);
    if (ReturnValue != 0)
    {
        DieWithUserMessage("getaddrinfo() failed", gai_strerror(ReturnValue));
    }

    int Socket = -1;
    for (struct addrinfo* AddressInfo = ServerAddressInfos; AddressInfo != NULL; AddressInfo = AddressInfo->ai_next)
    {
        Socket = socket(AddressInfo->ai_family, AddressInfo->ai_socktype, AddressInfo->ai_protocol);
        if (Socket < 0)
        {
            continue;
        }

        if (connect(Socket, AddressInfo->ai_addr, AddressInfo->ai_addrlen) == 0)
        {
            break;
        }

        close(Socket);
        Socket = -1;
    }

    freeaddrinfo(ServerAddressInfos);
    return Socket;
}