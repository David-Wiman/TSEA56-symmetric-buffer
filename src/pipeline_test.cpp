#include <iostream>
#include<stdarg.h>
#include<thread>
#include <atomic>
#include <chrono>

#include "pipeline.h"

#define STAGE_COMP_TIME 100

using namespace std;

struct my_data {
    int index{0};
    chrono::time_point<chrono::high_resolution_clock> timestamp{};
    vector<double> transfer_times{};
};

// The fetcher worker needs an internal state. This is done by implementing a
// callable class (with the operator() method).

class Fetcher {
public:
    Fetcher(): data_counter{0} {}
    void* operator()() {
        const auto start = chrono::high_resolution_clock::now();
        while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
        my_data *d = new my_data{};
        d->index = data_counter++;
        d->timestamp = chrono::high_resolution_clock::now();
        return d;
    }
private:
    int data_counter;
};

// If no state needed just use a function

void* worker(void *data) {
    // Measure data transfer time
    my_data *d = static_cast<my_data*>(data);
    const auto start_time = d->timestamp;
    const auto stop_time = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, std::milli>(stop_time-start_time).count();
    d->transfer_times.push_back(duration);

    // Do something
    const auto start = chrono::high_resolution_clock::now();
    while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());

    d->timestamp = chrono::high_resolution_clock::now();
    return data;
}


int main() {
    cout << "\nStart pipeline test" << endl;
    Pipeline pipeline{Fetcher{}, {worker, worker, worker}};
    for (int i{0}; i < 4; ++i) {
        my_data *d = static_cast<my_data*>(pipeline.get_future().get());
        cout << "Data " << i << " returned. Waited: "
             << d->transfer_times[0] << " ms, "
             << d->transfer_times[1] << " ms, "
             << d->transfer_times[2] << " ms"
             << endl;
        delete d;
    }
    cout << "\nEnd pipeline test" << endl;
}
