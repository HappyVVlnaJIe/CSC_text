#pragma once

#ifndef CLIENT_H
#define CLIENT_H

#endif // CLIENT_H


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

class DefaultClient
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    int error = 0;

    u_long iMode = 1;

    void InitializeWinSocket();
    void ConnectToServer(const char* server_address, const char* server_port);
    void Read();

public:
    DefaultClient(const char* server_address, const char* server_port,  u_long iMode);
    ~DefaultClient();
    bool Send(std::string message);
    std::string ReadFromServer();
    const int Status();
};
