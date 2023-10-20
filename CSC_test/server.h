#pragma once

#ifndef SERVER_H
#define SERVER_H

#endif // SERVER_H

#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <unordered_map>
#include "message.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

class DefaultServer
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    u_long iMode = 1;

    std::string signature;
    std::string separator = "@";
    int error = 0;

    void InitializeWinSocket(const char* port,  std::string signature);
    void CreateMasterSocket();
    void Bind();
    void Listen();
    void Read();

public:
    DefaultServer(const char* port,  std::string signature);
    virtual ~DefaultServer();
    void CheckClient();
    bool Send(std::string client_signature, std::string text);
    std::queue<Message> messages;
    const int Status();
};
