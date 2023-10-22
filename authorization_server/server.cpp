#include <utility>
#include <exception>
#include <sstream>
#include "server.h"


BaseServer::BaseServer(const char* port)
{
    InitializeWinSocket(port);
    CreateMasterSocket();
    Bind();
    LoadData();
    Listen();
    commands_names = {
        {"authorization", Commands::authorization},
        {"registration", Commands::registration}
    };
    std::cout << "create server" << std::endl;
}

void BaseServer::InitializeWinSocket(const char* port)
{
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cout<<("WSAStartup failed: {}\n", iResult);
        throw std::exception();
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0)
    {
        std::cout << ("getaddrinfo failed: {}\n", iResult);
        WSACleanup();
        throw std::exception();
    }
}

void BaseServer::CreateMasterSocket()
{
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        std::cout << ("socket failed: {}\n", iResult);
        freeaddrinfo(result);
        WSACleanup();
        throw std::exception();
    }

    iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        std::cout << ("ioctlsocket failed: {}\n", iResult);
        freeaddrinfo(result);
        WSACleanup();
        throw std::exception();
    }
}

void BaseServer::Bind()
{
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << ("bind failed: {}\n", iResult);
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        throw std::exception();
    }

    freeaddrinfo(result);
}

void BaseServer::LoadData()
{
    std::ifstream f(data_file_path.c_str());
    std::string s;
    while (std::getline(f, s)) 
    {
        int pos = s.find(" ");
        users_data[s.substr(0, pos)] = s.substr(pos + 1);
    }
    f.close();
}

void BaseServer::Listen()
{
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << ("listen failed: {}\n", iResult);
        closesocket(ListenSocket);
        WSACleanup();
        throw std::exception();
    }

}

std::string BaseServer::Read()
{
    std::string message = "";
    int last = 0;
    do
    {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            message += recvbuf;
            last += iResult;
        }
        else if (iResult == 0)
        {
            printf("Connection closing...\n");
            closesocket(ClientSocket);
            ClientSocket = INVALID_SOCKET;
        }
        else
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                closesocket(ClientSocket);
                WSACleanup();
            }
        }

    } while (iResult > 0);

    if (message != "")
    {
        message = message.substr(0, last);
    }
    return message;
}

std::string BaseServer::Authorization(std::istringstream& str_stream)
{
    std::string login, pass;
    str_stream >> login;
    str_stream >> pass;
    if (users_data.find(login) != users_data.end() && users_data[login] == pass)
    {
        return positive_response;
    }
    return negative_response;
}

std::string BaseServer::Registration(std::istringstream& str_stream)
{
    std::string login, pass;
    str_stream >> login;
    str_stream >> pass;
    if (users_data.find(login) != users_data.end())
    {
        return negative_response;
    }

    users_data[login] = pass;
    std::ofstream f;
    f.open(data_file_path.c_str(), std::ios::app);
    f << login << " " << pass << std::endl;
    f.close();
    return positive_response;
}

std::string BaseServer::ProcessingRequest(std::string request)
{
    int occurrences = 0;
    std::string::size_type pos = 0;
    while ((pos = request.find(" ", pos)) != std::string::npos) //TODO: пофиксить проблему с пробелами
    {
        ++occurrences;
        pos += 1;
    }
    if (occurrences != 3)
    {
        return wrong_command_response;
    }
    std::istringstream str_stream(request);
    std::string str_command;
    str_stream >> str_command;
    auto it = commands_names.find(str_command);
    if (it == commands_names.end())
    {
        return wrong_command_response;
    }
    Commands command = it->second;
    std::string response = "";
    switch (command)
    {
    case Commands::authorization:
        response = Authorization(str_stream);
        break;

    case Commands::registration:
        response = Registration(str_stream);
        break;

    default:

        break;
    }

    return response;
}

void BaseServer::Send(std::string response)
{
    iSendResult = send(ClientSocket, response.c_str(), response.size(), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        std::cout<< "accept failed with error: " + WSAGetLastError() << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
    }
}

void BaseServer::Run()
{
    while(true) 
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket != INVALID_SOCKET)
        {
            std::string request = Read();
            std::string response = ProcessingRequest(request);
            Send(response);
            closesocket(ClientSocket);
        }
    }
}

BaseServer::~BaseServer()
{
    closesocket(ListenSocket);
    WSACleanup();
}
