#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"

//information for linker to add winsock
#pragma comment (lib, "ws2_32.lib")

using namespace std;

class ChatClient {
private:
    SOCKET clientSocket;
    string username;
    bool running;

    //receiving messages from server
    void receiveMessages() {
        char buffer[MAX_MSG_LEN];   //buffer for data

        while(running) {
            int bytesReceived = recv(clientSocket, buffer, MAX_MSG_LEN, 0);
            if (bytesReceived <= 0) {       //disconnect or error
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
        //winsock initialization
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
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);       //creating socket tcp
        if (clientSocket == INVALID_SOCKET) {
            throw runtime_error("Socket creation failed");
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;         //ipv4
        serverAddress.sin_port = htons(DEFAULT_PORT);       //network port
        inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);  //ip address conversion

        //close socket if connetcion failed
        if (connect(clientSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            throw runtime_error("Connect failed");
        }

        send(clientSocket, username.c_str(), username.length(), 0);
        running = true;
        thread receiver(&ChatClient::receiveMessages, this);        //new thread to receive messages
        receiver.detach();

        cout << "Client connected" << endl;

        string message;
        while (running) {
            getline(cin, message);      //loading message from user
            send(clientSocket, message.c_str(), message.length(), 0);       //sending data to server
        }
    }

    void stop() {
        running = false;
        closesocket(clientSocket);
    }
};

int main() {
    try {
        string serverIP = "127.0.0.1";      //default localhost
        string username;

        cout << "Enter username: ";
        cin >> username;                    //client type username
        if (username.empty()) {
            username = "Anonymous";         //default name anonymous
        }
        ChatClient client;
        client.start(serverIP, username);       //start client connection
    } catch (const exception &e) {
        cerr << e.what() << endl;           //error handling
        return 1;
    }
    return 0;
}