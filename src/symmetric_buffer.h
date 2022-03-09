#ifndef SYMMETRIC_BUFFER_H
#define SYMMETRIC_BUFFER_H

#include <mutex>
#include <condition_variable>

#include <iostream>

/* The SymmetricBuffer class allows data transfer between different threads
 * using symmetric syncronization.
 *
 * Each SymmetricBuffer operates on one type T. That can be enything; for
 * example int, std::string or even a user defined class.
 *
 * A SymmetricBuffer b of type T is declared as SymmetricBuffer<T> b;
 *
 * A symmetric buffer of type T actually handles objects of type
 * std::unique_ptr<T>. That to ensure no copys and also no access to the same
 * object from multiple threads.
 *
 * The store and extract methods both block. store() will wait til some other
 * thread tries to extract the value and extract() will wait til the value is
 * available (some thread stores it).
 *
 * To use a buffer in different threads you need to give all threads access to
 * the same buffer object. Through a global variable or idealy a
 * pointer/refrence to the buffer.
 */

template <class T>
class SymmetricBuffer {
public:
    SymmetricBuffer<T>(): mtx{}, cv{}, has_data{false}, buffer{} {}

    SymmetricBuffer<T>(const SymmetricBuffer<T>&) = delete;
    SymmetricBuffer<T> operator=(const SymmetricBuffer) = delete;

    void store(std::unique_ptr<T> data);
    inline void store(T const &data) {
        store(std::make_unique<T>(data));
    }

    std::unique_ptr<T> extract();

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool has_data;
    std::unique_ptr<T> buffer;
};


template <class T>
void SymmetricBuffer<T>::store(std::unique_ptr<T> data) {
    {
        // Aquire lock
        std::lock_guard<std::mutex> lk(mtx);

        // Write in buffer
        buffer = std::move(data);
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
std::unique_ptr<T> SymmetricBuffer<T>::extract() {
    std::unique_ptr<T> data;
    {
        // Aquire lock
        std::unique_lock<std::mutex> lk(mtx);

        // Wait for data (unique_lock allows us to release the mutex while waiting)
        cv.wait(lk, [this]{return has_data;});

        // Read data
        data = std::move(buffer);
        has_data = false;
    }

    // Notify data read
    cv.notify_one();

    return data;
}


#endif  // SYMMETRIC_BUFFER_H
