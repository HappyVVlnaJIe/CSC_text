#include <QDebug>
#include <utility>

#include "server.h"


DefaultServer::DefaultServer(const char* port, std::string tech_message)
{
    error = 0;
    InitializeWinSocket(port);
    CreateMasterSocket();
    Bind();
    Listen();
    this->tech_message = tech_message;
}

void DefaultServer::InitializeWinSocket(const char* port)
{
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        error = WSAGetLastError();
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if ( iResult != 0 )
    {
        error = WSAGetLastError();
        WSACleanup();
        return;
    }
}

void DefaultServer::CreateMasterSocket()
{
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        error = WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        error = WSAGetLastError();
        return;
    }
}

void DefaultServer::Bind()
{
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    freeaddrinfo(result);
}

void DefaultServer::Listen()
{
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

}

void DefaultServer::CheckClient()
{
    if (ClientSocket != INVALID_SOCKET)
    {
        Read();
    }
    else
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            return;
        }
        else
        {
            std::string message = tech_message;
            iSendResult = send( ClientSocket, message.c_str(), message.size(), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return;
            }
            Read();
        }
    }
}

void DefaultServer::Read()
{
    std::string message = "";
    int last = 0;
    do
    {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            message += recvbuf;
            last+=iResult;
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
        message[last] = '\0';
        this->messages.push(message);
    }
}

bool DefaultServer::Send(std::string client_signature, std::string message)
{
    iSendResult = send( ClientSocket, message.c_str(), message.size(), 0 );
    if (iSendResult == SOCKET_ERROR)
    {
        qDebug(std::to_string(WSAGetLastError()).c_str());
        closesocket(ClientSocket);
        WSACleanup();
        return false;
    }
    return true;
}

DefaultServer::~DefaultServer()
{
    closesocket(ListenSocket);
    WSACleanup();
}

const int DefaultServer::Status()
{
    return error;
}
