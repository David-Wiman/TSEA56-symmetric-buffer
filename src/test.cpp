#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#include "logging.h"
#include "symmetric_buffer.h"

#define STAGE_COMP_TIME 1000  // Time (in ms) for the stage compute times

using namespace std;

atomic<bool> running{true};

// If internal memory is needed (in this case to count number of data produced)
// the worker function must be a class which implements the
// void operator(arg...)() method.
class Producer {
public:
    Producer(SymmetricBuffer<string> *buffer): buffer{buffer}, counter{0} {}
    void operator()(){
        print_with_lock("Start producer\n");

        while (running.load()) {
            // Do something
            const auto start = chrono::high_resolution_clock::now();
            while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
            print_with_lock("Producer done\n");
            string s{"Bar" + to_string(counter++)};
            buffer->store(s);
            // which is a shorthand for buffer->store(make_unique<string>(s));
        }
    }
private:
    SymmetricBuffer<string> *buffer;
    int counter;
};

void consumer(SymmetricBuffer<string> *buffer) {
    print_with_lock("Start consumer\n");

    while (running.load()) {
        // Do something
        const auto start = chrono::high_resolution_clock::now();
        while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
        print_with_lock("Consumer done\n");

        auto msg = buffer->extract();
        cout << "Message was: " << *msg << endl;
    }
}

int main() {
    cout << "\nStart pipeline test" << endl;

    SymmetricBuffer<string> buffer{};
    thread t1{Producer{&buffer}};
    thread t2{consumer, &buffer};

    this_thread::sleep_for(chrono::milliseconds(STAGE_COMP_TIME*5));
    running.store(false);

    t1.join();
    t2.join();


    cout << "\nEnd pipeline test" << endl;
}
