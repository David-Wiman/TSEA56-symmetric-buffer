#include <iostream>
#include<stdarg.h>
#include<thread>
#include <atomic>
#include <chrono>

#include "pipeline.h"

#define STAGE_COMP_TIME 3000

using namespace std;

struct my_data {
    int index{0};
    chrono::time_point<chrono::high_resolution_clock> timestamp{};
    vector<double> transfer_times{};
};

atomic<int> data_counter{0};

void* fetch_data() {
    const auto start = chrono::high_resolution_clock::now();
    while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
    int counter = data_counter.load();
    data_counter.store(counter + 1);
    my_data *d = new my_data{};
    d->index = counter;
    d->timestamp = chrono::high_resolution_clock::now();
    return d;
}

void* worker1(void *data) {
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

void* worker2(void *data) {
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

void* worker3(void *data) {
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
    Pipeline pipeline{fetch_data, {worker1, worker2, worker3}};
    for (int i{0}; i < 10; ++i) {
        my_data *d = static_cast<my_data*>(pipeline.get_future().get());
        cout << "Data " << i << " returned. Waited: "
             << d->transfer_times[0] << " ms, "
             << d->transfer_times[1] << " ms, "
             << d->transfer_times[2] << " ms"
             << endl;
    }
    cout << "\nEnd pipeline test" << endl;
}
