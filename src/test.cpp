#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "logging.h"
#include "symmetric_buffer.h"

#define STAGE_COMP_TIME 1000  // Time (in ms) for the stage compute times

using namespace std;

atomic<bool> running{true};

// If internal memory is needed (in this case to count number of data produces)
// the worker funktion must be a class which implements the
// void operator(arg...)() method.
class Producer {
public:
    Producer(SymmetricBuffer<string*> *buffer): buffer{buffer}, counter{0} {}
    void operator()(){
        print_with_lock("Start producer\n");

        while (running.load()) {
            // Do something
            const auto start = chrono::high_resolution_clock::now();
            while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
            print_with_lock("Producer done\n");

            auto s = new string{"Foo" + to_string(counter++)};
            buffer->store(s);
        }
    }
private:
    SymmetricBuffer<string*> *buffer;
    int counter;
};

void consumer(SymmetricBuffer<string*> *buffer) {
    print_with_lock("Start consumer\n");

    while (running.load()) {
        // Do something
        const auto start = chrono::high_resolution_clock::now();
        while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
        print_with_lock("Consumer done\n");

        auto s = buffer->extract();
        cout << "Message was: " << *s << endl;
        delete s;
    }
}

int main() {
    cout << "\nStart pipeline test" << endl;

    SymmetricBuffer<string*> buffer{};
    thread t1{Producer{&buffer}};
    thread t2{consumer, &buffer};

    this_thread::sleep_for(chrono::milliseconds(STAGE_COMP_TIME*5));
    running.store(false);

    t1.join();
    t2.join();


    cout << "\nEnd pipeline test" << endl;
}
