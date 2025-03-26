#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;

enum class PhilosopherState {
    THINKING,
    HUNGRY,
    EATING
};

int randomDelay(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

class DiningPhilosophers {
private:
    int numOfPhilosophers;
    vector<PhilosopherState> states;
    vector<unique_ptr<mutex>> forks;
    mutex consoleMutex;

    void printState(int philosopherID, const string &action) {
        lock_guard<mutex> lock(consoleMutex);
        cout << "Philosopher " << philosopherID << " " << action << endl;
    }

    void philosopherBehavior(int ID) {
        while (true) {
            //thinking
            states[ID] = PhilosopherState::THINKING;
            printState(ID, "is thinking...");
            this_thread::sleep_for(chrono::milliseconds(randomDelay(1000, 10000)));

            //hungry
            states[ID] = PhilosopherState::HUNGRY;
            printState(ID, "is hungry");

            // forks picked up in order to prevent deadlock
            int firstFork = min(ID, (ID+1) % numOfPhilosophers);
            int secondFork = max(ID, (ID+1) % numOfPhilosophers);

            forks[firstFork]->lock();
            printState(ID, "picked up fork" + to_string(firstFork));

            forks[secondFork]->lock();
            printState(ID, "picked up fork" + to_string(secondFork));

            // eating
            states[ID] = PhilosopherState::EATING;
            printState(ID, "is eating");
            this_thread::sleep_for(chrono::milliseconds(randomDelay(1000, 10000)));

            // put down forks when stops eating
            forks[secondFork]->unlock();
            printState(ID, "put down fork" + to_string(secondFork));

            forks[firstFork]->unlock();
            printState(ID, "put down fork" + to_string(firstFork));
        }
    }

public:
    DiningPhilosophers(int n):numOfPhilosophers(n), states(n, PhilosopherState::THINKING) {
        for(int i = 1; i < n+1; i++) {
            forks.emplace_back(make_unique<mutex>());
        }
    }
    ~DiningPhilosophers() {}

    void start() {
        vector<thread> philosopherThreads;
        for (int i = 1; i < numOfPhilosophers+1; i++) {
            philosopherThreads.emplace_back(thread(philosopherBehavior, i));
        }
        for (auto& pt : philosopherThreads) {
            pt.join();
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <numOfPhilosophers>" << endl;
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 2) {
        cout << "Number of Philosophers must be greater than 2" << endl;
        return 1;
    }

    cout << "Starting dining philosophers program with " << n << " philosophers" << endl;
    DiningPhilosophers diningPhilosophers(n);
    diningPhilosophers.start();

    return 0;
}