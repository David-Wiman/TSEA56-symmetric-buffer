#include <iostream>
#include<stdarg.h>
#include<thread>
#include <atomic>
#include <chrono>

#include "pipeline.h"

#define STAGE_COMP_TIME 3000

using namespace std;

atomic<int> data_counter{0};

void* fetch_data() {
    const auto start = chrono::high_resolution_clock::now();
    while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());
    int counter = data_counter.load();
    string *s = new string("Data" + to_string(counter) + "_");
    data_counter.store(counter + 1);
    return s;
}

void* worker1(void *data) {

    // Do something
    const auto start = chrono::high_resolution_clock::now();
    while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());

    string &work = *static_cast<string*>(data);
    work = work + "A";
    return data;
}

void* worker2(void *data) {

    // Do something
    const auto start = chrono::high_resolution_clock::now();
    while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());

    string &work = *static_cast<string*>(data);
    work = work + "B";
    return data;
}

void* worker3(void *data) {

    // Do something
    const auto start = chrono::high_resolution_clock::now();
    while (start + chrono::milliseconds(STAGE_COMP_TIME) > chrono::high_resolution_clock::now());

    string &work = *static_cast<string*>(data);
    work = work + "C";
    return data;
}

int main() {
    cout << "\nStart pipeline test" << endl;
    Pipeline pipeline{fetch_data, {worker1, worker2, worker3}};
    //while (true) {}
    this_thread::sleep_for(chrono::milliseconds(24010));
    cout << "\nEnd pipeline test" << endl;
}
