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
#include <string>
#include <iostream>
#include <fstream>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

enum class Commands
{
    authorization,
    registration
};

class BaseServer
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    u_long iMode = 1;

    std::unordered_map<std::string, Commands> commands_names;
    std::unordered_map<std::string, std::string> users_data;
    std::string positive_response = "OK";
    std::string negative_response = "NOT";
    std::string wrong_command_response = "wrong command";
    std::string data_file_path = "../../../data.txt";

    void InitializeWinSocket(const char* port);
    void CreateMasterSocket();
    void Bind();
    void Listen();
    std::string Read();
    std::string ProcessingRequest(std::string request);
    void Send(std::string response);
    std::string BaseServer::Authorization(std::istringstream& str_stream);
    std::string BaseServer::Registration(std::istringstream& str_stream);
    void LoadData();

public:
    BaseServer(const char* port);
    virtual ~BaseServer();
    void Run();
};