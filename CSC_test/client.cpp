#include "client.h"
#include<QDebug>

DefaultClient::DefaultClient(const char* server_address, const char* server_port,  std::string signature)
{
    error = 0;
    InitializeWinSocket(signature);
    ConnectToServer(server_address, server_port);
}

void DefaultClient::InitializeWinSocket(std::string signature)
{
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        error = WSAGetLastError();;
        return;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    this->signature = signature;
}

void DefaultClient::ConnectToServer(const char* server_address, const char* server_port)
{
    iResult = getaddrinfo(server_address, server_port, &hints, &result);
    if ( iResult != 0 )
    {
        error = WSAGetLastError();
        WSACleanup();
        return;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            error = WSAGetLastError();
            WSACleanup();
            return;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            error = WSAGetLastError();
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        error = WSAGetLastError();
        WSACleanup();
        return;
    }

    iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        error = WSAGetLastError();
        return;
    }
}

bool DefaultClient::Send(std::string text)
{
    std::string message = text + separator + signature;
    iResult = send( ConnectSocket, message.c_str(), message.size(), 0 );
    if (iResult == SOCKET_ERROR)
    {
        error = WSAGetLastError();
        closesocket(ConnectSocket);
        WSACleanup();
        return false;
    }
    return true;
}

Message* DefaultClient::ReadFromServer()
{
    std::string message = "";
    int last = 0;
    do
    {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
        {
            message += recvbuf;
            last += iResult;
        }
        else if ( iResult == 0 )
        {

            printf("Connection closed\n");
        }
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }

    } while( iResult > 0 );

    if (message == "") return nullptr;

    message[last] = '\0';
    return  new Message(ParseMessage(message, separator));
}

DefaultClient::~DefaultClient()
{
    closesocket(ConnectSocket);
    WSACleanup();
}

const int DefaultClient::Status()
{
    return error;
}

