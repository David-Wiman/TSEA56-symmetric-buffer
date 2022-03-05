#ifndef SYMMETRIC_BUFFER_H
#define SYMMETRIC_BUFFER_H

#include <mutex>
#include <condition_variable>

#include <iostream>

template <class T>
class SymmetricBuffer {
public:
    SymmetricBuffer<T>(): mtx{}, cv{}, has_data{false}, buffer{} {}

    SymmetricBuffer<T>(const SymmetricBuffer<T>&) = delete;
    SymmetricBuffer<T> operator=(const SymmetricBuffer) = delete;

    void store(T data);

    T extract();

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool has_data;
    T buffer;
};


template <class T>
void SymmetricBuffer<T>::store(T data) {
    {
        // Aquire lock
        std::lock_guard<std::mutex> lk(mtx);

        // Write in buffer
        buffer = data;
        has_data = true;
    }

    // Notify data written
    cv.notify_one();

    // Wait for read
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [this]{return !has_data;});
    }
}

template <class T>
T SymmetricBuffer<T>::extract() {
    T data;
    {
        // Aqure lock
        std::unique_lock<std::mutex> lk(mtx);

        // Wait for data (unique_lock allows us to release the mutex while waiting)
        cv.wait(lk, [this]{return has_data;});

        // Read data
        data = buffer;
        has_data = false;
    }

    // Notify data read
    cv.notify_one();

    return data;
}



#endif  // SYMMETRIC_BUFFER_H
