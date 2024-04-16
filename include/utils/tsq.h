#ifndef UTILS_TSQ_H
#define UTILS_TSQ_H

template <typename T>
class TSQ {
private:

    std::deque<T> q;
    std::mutex q_write;

public:

    void push(T t) {
        q_write.lock();
        q.push_back(t);
        q_write.unlock();
    }

    bool pushAsync(T t) {
        if (q_write.try_lock()) {
            q.push_back(t);
            return true;
        }
        return false;
    }

    T top() {
        while (q.empty()) { }
        return q.top();
    }

    T pop() {
        while (true) {
            if (q.empty()) continue;
            q_write.lock();
            if (!q.empty()) break;
            q_write.unlock();
        }
        T temp = q.front();
        q.pop_front();
        q_write.unlock();
        return temp;
    }
};

#endif
