#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include "common.h"

//information for linker to add winsock
#pragma comment(lib, "ws2_32.lib")

using namespace std;

class ChatServer {
private:
    SOCKET serverSocket;
    vector<SOCKET> clientSockets;
    vector<thread> clientThreads;
    MessageQueue messageQueue;
    mutex clientMutex;
    bool running;


    // generating timestamp for messages
    string getCurrentTimestamp() {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << ctime(&time);
        string ts = ss.str();

        // deleting new line from timestamp
        if(!ts.empty() && ts.back() == '\n') {
            ts.pop_back();
        }

        return ts;
    }

    // sending messages to all clients
    void broadcastMessage(const Message& msg) {
        lock_guard<mutex> lock(clientMutex);    //blocking mutex
        //formating message
        string formattedMessage = "[" + msg.timestamp + "]" + msg.sender + ": " + msg.content + "\n";

        //sending message to all connected clients
        for (SOCKET clientSocket : clientSockets) {
            send(clientSocket, formattedMessage.c_str(), formattedMessage.length(), 0);
        }
    }

    // handling single client
    void handleClient(SOCKET clientSocket) {

        //assign clients name
        char nameBuffer[MAX_MSG_LEN];       //buffer for received data
        int nameBytes = recv(clientSocket, nameBuffer, MAX_MSG_LEN-1, 0);
        if (nameBytes <= 0) {
            return;
        }
        nameBuffer[nameBytes] = '\0';       //end of string
        string clientName = string(nameBuffer);

        char buffer[MAX_MSG_LEN];

        while (running) {
            int bytesReceived = recv(clientSocket, buffer, MAX_MSG_LEN-1, 0);
            if (bytesReceived <= 0) {
                break;
            }
            buffer[bytesReceived] = '\0';
            //creating message structure
            Message msg{clientName, string(buffer), getCurrentTimestamp()};
            messageQueue.push(msg);     //adding message to queue
        }
        //remove client when disconnected
        {
            lock_guard<mutex> lock(clientMutex);
            auto it = find(clientSockets.begin(), clientSockets.end(), static_cast<SOCKET>(clientSocket));
            if (it != clientSockets.end()) {
                clientSockets.erase(it);
            }
        }
        closesocket(clientSocket);
    }

    void messageBroadcaster() {
         while (running) {
             Message msg;
             if (messageQueue.pop(msg)) {
                 broadcastMessage(msg);
             }
         }
    }

public:
    ChatServer() : running(false) {     //server initialization
        WSADATA wsaData;                //storage data about winsock version
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("WSAStartup failed");
        }
    }

    ~ChatServer() {
        stop();
        WSACleanup();
    }

    void start() {
        //creating tcp/ip socket
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //ipv4, tcp
        if (serverSocket == INVALID_SOCKET) {
            throw runtime_error("socket creationm failed");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;                         //ipv4
        serverAddress.sin_addr.s_addr = INADDR_ANY;                 //listen on all available interfaces
        serverAddress.sin_port = htons(DEFAULT_PORT);

        //assign ip address and port to socket
        if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            throw runtime_error("bind failed");
        }

        //set socket into listening mode
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(serverSocket);
            throw runtime_error("listen failed");
        }

        running = true;
        //create separate thread to send messages
        thread broadcaster(&ChatServer::messageBroadcaster, this);
        broadcaster.detach();

        cout << "Server listening on port " << DEFAULT_PORT << endl;

        while (running) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                continue;
            }

            {
                lock_guard<mutex> lock(clientMutex);
                clientSockets.push_back(clientSocket);      //add new client to list
            }
            clientThreads.emplace_back(&ChatServer::handleClient, this, clientSocket);      //new thread to client service
        }
    }

    void stop() {
        running = false;

        //close all connections
        {
            lock_guard<mutex> lock(clientMutex);
            for (SOCKET clientSocket : clientSockets) {
                closesocket(clientSocket);
            }
            clientSockets.clear();
        }

        //clear all threads and close server socket
        for (auto& thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        clientThreads.clear();
        closesocket(serverSocket);
    }
};

int main() {
    try {
        ChatServer server;
        server.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
    return 0;
}
