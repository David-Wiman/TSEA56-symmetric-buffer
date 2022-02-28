#ifndef LOGGING_H
#define LOGGING_H

#include <mutex>

#include "globals.h"

std::mutex print_mtx;

#define print_with_lock(fmt, args...)({lock_guard<mutex> guard(print_mtx); printf(fmt, ##args);})

#ifdef LOG_WORKERS
#   define log_worker(fmt, args...)(print_with_lock(fmt, ##args))
#else
#   define log_worker(fmt, args...)({})
#endif

#endif  // LOGGING_H
