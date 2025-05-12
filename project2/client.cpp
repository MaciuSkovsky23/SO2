#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

class ChatClient {
private:
    SOCKET clientSocket;
    string username;
    bool running;

    void receiveMessages() {
        char buffer[MAX_MSG_LEN];
        while(running) {
            int bytesReceived = recv(clientSocket, buffer, MAX_MSG_LEN, 0);
            if (bytesReceived <= 0) {
                cout << "Client disconnected" << endl;
                running = false;
                break;
            }
            buffer[bytesReceived] = '\0';
            cout << buffer << endl;
        }
    }

public:
    ChatClient() : running(false) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw runtime_error("WSAStartup failed");
        }
    }

    ~ChatClient() {
        stop();
        WSACleanup();
    }

    void start(const string &serverIP, const string &name) {
        username = name;
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            throw runtime_error("Socket creation failed");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(DEFAULT_PORT);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

        if (connect(clientSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            throw runtime_error("Connect failed");
        }
        running = true;
        thread receiver(&ChatClient::receiveMessages, this);
        receiver.detach();

        cout << "Client connected" << endl;

        string message;
        while (running) {
            getline(cin, message);
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }

    void stop() {
        running = false;
        closesocket(clientSocket);
    }
};

int main() {
    try {
        string serverIP = "127.0.0.1";
        string username;

        cout << "Enter username: ";
        cin >> username;
        if (username.empty()) {
            username = "Anonymous";
        }
        ChatClient client;
        client.start(serverIP, username);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}