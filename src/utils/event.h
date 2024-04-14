#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <list>
#include <thread>
#include <cassert>
#include <functional>
#include <type_traits>

namespace event {

template <typename EventType>
using Listener = std::function<void(EventType)>;

template <typename EventType>
struct ListenerHandle {
    typename std::list<Listener<EventType>>::const_iterator iter;
    bool valid;
};

template <typename EventType>
struct Impl {
    using ListenerList = std::list<Listener<EventType>>;
    static ListenerList listeners;

    static void triggerImpl(EventType e) {
        for (auto &&l : listeners) {
            std::invoke(l, e);
        }
    }
};
template <typename EventType>
typename Impl<EventType>::ListenerList Impl<EventType>::listeners = ListenerList();

template <typename EventType>
void trigger(EventType &&e) {
    std::thread t(Impl<EventType>::triggerImpl, std::forward<EventType>(e));
    t.join();
}

template <typename EventType>
ListenerHandle<EventType> registerListener(Listener<EventType> &&l) {
    Impl<EventType>::listeners.push_back(std::forward<Listener<EventType>>(l));
    return ListenerHandle<EventType>{--Impl<EventType>::listeners.end(), true};
}
template <typename EventType>
ListenerHandle<EventType> registerListener(EventType, Listener<EventType> &&l) {
    return registerListener<EventType>(std::forward<Listener<EventType>>(l));
}

template <typename EventType>
void deregisterListener(ListenerHandle<EventType> &lh) {
    assert(lh.valid == true && "invalid listener ref");
    Impl<EventType>::listeners.erase(lh.iter);
    lh.valid = false;
}
template <typename EventType>
void deregisterListener(EventType, ListenerHandle<EventType> &lh) {
    deregisterListener(lh);
}

} // namespace event

#endif
