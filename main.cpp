#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;

//enum to represent states philosophers can be in
enum class PhilosopherState {
    THINKING,
    HUNGRY,
    EATING
};

//Function to get random time for thinking and eating
int randomDelay(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

class DiningPhilosophers {
private:
    int numOfPhilosophers;              //number of philosophers (argument)
    vector<PhilosopherState> states;    //states of philosophers
    vector<unique_ptr<mutex>> forks;    //mutexes for forks
    mutex consoleMutex;                 //mutex for console output

    //function to print a philosopher state
    void printState(int philosopherID, const string &action) {
        lock_guard<mutex> lock(consoleMutex);   //avoid mess in console output
        cout << "Philosopher " << philosopherID + 1 << " " << action << endl;
    }

    //function to represent behavior of a philosopher
    void philosopherBehavior(int ID) {
        while (true) {
            //thinking
            states[ID] = PhilosopherState::THINKING;
            printState(ID, "is thinking...");
            this_thread::sleep_for(chrono::milliseconds(randomDelay(1000, 10000)));

            //when state changes to hungry philosopher try to pick up forks
            states[ID] = PhilosopherState::HUNGRY;
            printState(ID, "is hungry");

            // forks picked up in order to prevent deadlock (lower id first)
            int firstFork = min(ID, (ID+1) % numOfPhilosophers);
            int secondFork = max(ID, (ID+1) % numOfPhilosophers);

            // try to get the first fork
            forks[firstFork]->lock();
            printState(ID, "picked up fork " + to_string(firstFork + 1));

            //try to get the second fork
            //if its taken wait with first fork in hand
            forks[secondFork]->lock();
            printState(ID, "picked up fork " + to_string(secondFork + 1));

            // eating
            states[ID] = PhilosopherState::EATING;
            printState(ID, "is eating");
            this_thread::sleep_for(chrono::milliseconds(randomDelay(1000, 10000)));

            // put down forks in reverse when stops eating
            printState(ID, "put down fork " + to_string(secondFork + 1));
            forks[secondFork]->unlock();

            printState(ID, "put down fork " + to_string(firstFork + 1));
            forks[firstFork]->unlock();
        }
    }

public:
    DiningPhilosophers(int n):numOfPhilosophers(n), states(n, PhilosopherState::THINKING) {
        //initialize forks mutexes
        for(int i = 0; i < n; i++) {
            forks.emplace_back(make_unique<mutex>());   //create unique mutex for each fork
        }
    }
    ~DiningPhilosophers() {}

    //function to start philosophers threads
    void start() {
        vector<thread> philosopherThreads;
        //create philosopher threads
        for (int i = 0; i < numOfPhilosophers; i++) {
            philosopherThreads.emplace_back([this, i]()-> void {philosopherBehavior(i);});
        }
        //join threads
        //threads run indefinitely until program is terminated
        for (auto& pt : philosopherThreads) {
            pt.join();
        }
    }
};

int main(int argc, char *argv[]) {
    //check if correct number of arguments
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <numOfPhilosophers>" << endl;
        return 1;
    }
    //convert argument to int
    int n = atoi(argv[1]);

    //check if number of philosophers is correct
    if (n < 2) {
        cout << "Number of Philosophers must be greater than 2" << endl;
        return 1;
    }

    cout << "Starting dining philosophers program with " << n << " philosophers" << endl;

    //create instance of diningphilosophers class and start simulation
    DiningPhilosophers diningPhilosophers(n);
    diningPhilosophers.start();

    return 0;
}