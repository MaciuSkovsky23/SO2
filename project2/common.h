#ifndef COMMON_H
#define COMMON_H

using namespace std;

const int DEFAULT_PORT = 8080;
const int MAX_MSG_LEN = 1024;
const int MAX_CLIENTS = 10;

struct Message {
    string timestamp;
    string sender;
    string content;
};

class MessageQueue {
private:

public:
    void push(const Message & msg);

    bool pop(const Message & msg);
};

#endif //COMMON_H
