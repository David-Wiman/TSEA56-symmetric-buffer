#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdarg>

#include "pipeline.h"


using namespace std;

mutex print_mtx;

#define print_with_lock(fmt, args...)({lock_guard<mutex> guard(print_mtx); printf(fmt, ##args);})

FirstStage::FirstStage(void* (*func)(), pipeline_link &link_out)
: GenericStage(link_out), func{func} {
    stage_index = 0;
    thread t(&FirstStage::run, this);
    t.detach();
}

Stage::Stage(void* (*func)(void*), pipeline_link &link_in, pipeline_link &link_out, unsigned int stage_idx)
: GenericStage(link_out), link_in{link_in}, func{func} {
    stage_index = stage_idx;
    thread t(&Stage::run, this);
    t.detach();
}

void FirstStage::run() {
    while (true) {
        print_with_lock("Start worker %d\n", stage_index);
        const auto start_time = chrono::high_resolution_clock::now();
        void* data = func();
        const auto stop_time = chrono::high_resolution_clock::now();
        double duration = chrono::duration<double, std::milli>(stop_time-start_time).count();
        print_with_lock("Worker %d ran for %.2f ms\n", stage_index, duration);
        {
            unique_lock<mutex> lk(link_out.mtx);
            link_out.cv.wait(lk, [this]{return !link_out.has_data;});
            link_out.buffer = data;
            link_out.has_data = true;
        }
        link_out.cv.notify_one();
    }
}

void Stage::run() {
    while (true) {
        void* data;
        const auto start_wait = chrono::high_resolution_clock::now();
        {
            unique_lock<mutex> lk(link_in.mtx);
            link_in.cv.wait(lk, [this]{return link_in.has_data;});
            data = link_in.buffer;
            link_in.has_data = false;
        }
        link_in.cv.notify_one();
        const auto stop_wait = chrono::high_resolution_clock::now();
        double wait_dur = chrono::duration<double, std::milli>(stop_wait-start_wait).count();

        print_with_lock("Start worker %d (waited %.2f ms)\n", stage_index, wait_dur);
        const auto start_work = chrono::high_resolution_clock::now();
        data = func(data);
        const auto stop_work = chrono::high_resolution_clock::now();
        double work_dur = chrono::duration<double, std::milli>(stop_work-start_work).count();
        print_with_lock("Worker %d ran for %.2f ms\n", stage_index, work_dur);

        {
            unique_lock<mutex> lk(link_out.mtx);
            link_out.cv.wait(lk, [this]{return !link_out.has_data;});
            link_out.buffer = data;
            link_out.has_data = true;
        }
        link_out.cv.notify_one();
    }
}

