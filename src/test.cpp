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

atomic<bool> run_producer{true};
atomic<bool> run_consumer{true};

// If internal memory is needed (in this case to count number of data produced)
// the worker function must be a class which implements the
// void operator(arg...)() method.
class Producer {
public:
    Producer(SymmetricBuffer<string> *buffer): buffer{buffer}, counter{0} {}
    void operator()(){

        while (run_producer.load()) {
            print_with_lock("Start producer loop #%d\n", counter);
            // Do something
            const auto start = chrono::high_resolution_clock::now();
            while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
            print_with_lock("Producer done\n");
            string s{"Bar" + to_string(counter++)};
            buffer->store(s);
            // which is a shorthand for buffer->store(make_unique<string>(s));
        }
        print_with_lock("Shutdown producer\n");
    }
private:
    SymmetricBuffer<string> *buffer;
    int counter;
};

void consumer(SymmetricBuffer<string> *buffer_in, SymmetricBuffer<string> *buffer_out) {

    while (run_producer.load() || buffer_in->has_data()) {
        print_with_lock("Resart consumer\n");

        // Extact data
        auto msg = buffer_in->extract();

        // Do something
        const auto start = chrono::high_resolution_clock::now();
        while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());

        *msg = *msg + "_processed";

        print_with_lock("Consumer done\n");
        buffer_out->store(std::move(msg));
    }
    print_with_lock("Shutdown consumer\n");
}

int main() {
    cout << "\nStart pipeline test" << endl;

    SymmetricBuffer<string> buffer0{};
    SymmetricBuffer<string> buffer1{};
    thread t1{Producer{&buffer0}};
    thread t2{consumer, &buffer0, &buffer1};

    for (int i{0}; i < 5; i++) {
        auto data = buffer1.extract();
        cout << "Recieved data: " << *data << endl;
    }

    // Shutdown
    run_producer.store(false);
    cout << "End of loop, processing remaining data" << endl;
    auto data = buffer1.extract();
    t1.join();
    cout << "Recieved data: " << *data << endl;
    t2.join();

    cout << "\nEnd pipeline test" << endl;
}
