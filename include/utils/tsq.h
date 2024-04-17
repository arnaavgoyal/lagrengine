#ifndef UTILS_TSQ_H
#define UTILS_TSQ_H

#include <deque>
#include <limits.h>
#include <mutex>
#include <semaphore>

template <typename T>
class TSQ {
private:
    
    static constexpr unsigned max = UINT_MAX;
    std::deque<T> q;
    std::mutex q_write;
    std::counting_semaphore<max> q_sema;
    unsigned q_counter;

public:
    
    TSQ() : q_sema(0), q_counter(0) { }

    void push(T t) {
        q_write.lock();
        q.push_back(t);
        q_counter++;
        q_write.unlock();
        q_sema.release();
    }

    bool pushAsync(T t) {
        if (q_write.try_lock()) {
            q.push_back(t);
            q_counter++;
            q_write.unlock();
            q_sema.release();
            return true;
        }
        return false;
    }

    T top() {
        q_sema.acquire();
        T val = q.top();
        q_sema.release();
    }

    T pop() {
        q_sema.acquire();
        q_write.lock();
        T temp = q.front();
        q.pop_front();
        q_counter--;
        q_write.unlock();
        return temp;
    }

};

#endif
