#ifndef LOGGING_H
#define LOGGING_H

#include <mutex>

extern std::mutex global_print_mtx;

#define print_with_lock(fmt, args...)({lock_guard<mutex> guard(global_print_mtx); printf(fmt, ##args);})

#endif  // LOGGING_H
