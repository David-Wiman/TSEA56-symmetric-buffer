#include "logging.h"
#include <mutex>

std::mutex global_print_mtx{};
