#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

int main()
{
    // Initialze winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0)
    {
        cerr << "Can't Initialize winsock! Quitting" << endl;
        return 99;
    }

    // Create a socket
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET)
    {
        cerr << "Can't create a socket! Quitting" << endl;
        return 99;
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton ....

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Tell Winsock the socket is for listening
    listen(listening, SOMAXCONN);

    // Create the master file descriptor set and zero it
    fd_set master;
    FD_ZERO(&master);

    // Add our first socket that we're interested in interacting with; the listening socket!
    // It's important that this socket is added for our server or else we won't 'hear' incoming
    // connections
    FD_SET(listening, &master);

    // Wait for a connection
    sockaddr_in client;
    int clientSize = sizeof(client);

    SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

    char host[NI_MAXHOST];              // Clinet's remote name
    char service[NI_MAXSERV];           // Service (1.e. port) the clinet is connect on

    ZeroMemory(host, NI_MAXHOST);       // same as memset(host, 0, IN_MAXHOST);
    ZeroMemory(service, NI_MAXSERV);

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        cout << host << " connection on port " << service << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on port " <<
            ntohs(client.sin_port) << endl;
    }

    // Close listening socket
    closesocket(listening);

    // While loop: accept and echo message back to client
    char buf[4096];
    bool running = true;

    while (running)
    {
        // Make a copy of the master file descriptor set, this is SUPER important because
        // the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
        // are accepting inbound connection requests OR messages.

        // E.g. You have a server and it's master file descriptor set contains 5 items;
        // the listening socket and four clients. When you pass this set into select(),
        // only the sockets that are interacting with the server are returned. Let's say
        // only one client is sending a message at that time. The contents of 'copy' will
        // be one socket. You will have LOST all the other sockets.

        // SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

        fd_set copy = master;

        // See who's talking to us
        int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

        // Loop through all the current connections / potential connect
        for (int i = 0; i < socketCount; i++)
        {
            // Assignment to sock
            SOCKET sock = copy.fd_array[i];
        }

        ZeroMemory(buf, 4096);
        // Wait for client to send data
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        if (bytesReceived == 0)
        {
            cout << "Client disconnected " << endl;
            break;
        }

        // Echo message back to client
        send(clientSocket, buf, bytesReceived + 1, 0);
    }

    // Close the sock
    closesocket(clientSocket);

    // Shutdown winsock
    WSACleanup();
}