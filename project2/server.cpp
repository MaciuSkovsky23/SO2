#include <format>
#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <sstream>
#include "common.h"

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


    string getCurrentTimestamp() {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << ctime(&time);
        return ss.str();
    }

    void broadcastMessage(const Message& msg) {
        lock_guard<mutex> lock(clientMutex);
        string formattedMessage = "[" + msg.timestamp + "] " + msg.sender + ": " + msg.content + "\n";

        for (SOCKET clientSocket : clientSockets) {
            send(clientSocket, formattedMessage.c_str(), formattedMessage.length(), 0);
        }
    }

    void handleClient(SOCKET clientSocket) {
        char buffer[MAX_MSG_LEN];
        string clientName = "Client" + to_string(clientSocket);

        while (running) {
            int bytesReceived = recv(clientSocket, buffer, MAX_MSG_LEN, 0);
            if (bytesReceived <= 0) {
                break;
            }
            buffer[bytesReceived] = '\0';
            Message msg{clientName, string(buffer), getCurrentTimestamp()};
            messageQueue.push(msg);
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
    ChatServer() : running(false) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("WSAStartup failed");
        }
    }

    ~ChatServer() {
        stop();
        WSACleanup();
    }

    void start() {
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            throw runtime_error("socket creationm failed");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(DEFAULT_PORT);

        //if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) != 0) {
        if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            throw runtime_error("bind failed");
        }
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(serverSocket);
            throw runtime_error("listen failed");
        }

        running = true;
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
                clientSockets.push_back(clientSocket);
            }
            clientThreads.emplace_back(&ChatServer::handleClient, this, clientSocket);
        }
    }

    void stop() {
        running = false;

        {
            lock_guard<mutex> lock(clientMutex);
            for (SOCKET clientSocket : clientSockets) {
                closesocket(clientSocket);
            }
            clientSockets.clear();
        }
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
