#ifndef COMMON_H
#define COMMON_H
#include <condition_variable>
#include <queue>

using namespace std;

const int DEFAULT_PORT = 8080;          //port for listening
const int MAX_MSG_LEN = 1024;           //max message 1 KB

//message structure
struct Message {
    string timestamp;
    string sender;
    string content;
};

class MessageQueue {
private:
    mutex mtx;
    queue<Message> messages;
    condition_variable cv;

public:
    void push(const Message & msg) {
        lock_guard<mutex> lock(mtx);
        messages.push(msg);
        cv.notify_all();
    }

    bool pop(Message & msg) {
        lock_guard<mutex> lock(mtx);
        if (messages.empty()) {
            return false;
        }
        msg = messages.front();
        messages.pop();
        return true;
    }
};

#endif //COMMON_H
