#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <cassert>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

namespace event {

/**
 * Listener function callback type
 * @tparam EventType the event this listener is for
 */
template <typename EventType>
using Listener = std::function<void(EventType)>;

/**
 * Listener handle type
 * @tparam EventType the event this handle is for
 */
template <typename EventType>
struct ListenerHandle {
    typename std::list<Listener<EventType>>::const_iterator iter;
    bool valid;
};

/**
 * Implements event logic
 * @tparam EventType the event this impl is for
 */
template <typename EventType>
struct Impl {
    using ListenerList = std::list<Listener<EventType>>;
    static ListenerList listeners;
    static std::mutex mut;
    static std::condition_variable cond;

    static void triggerImpl(EventType e) {
        mut.lock();
        mut.unlock();
        cond.notify_all();
        for (auto &&l : listeners) {
            std::invoke(l, e);
        }
    }
};
template <typename EventType>
typename Impl<EventType>::ListenerList Impl<EventType>::listeners;
template <typename EventType>
std::mutex Impl<EventType>::mut;
template <typename EventType>
std::condition_variable Impl<EventType>::cond;

void trigger();

/**
 * Trigger events, notifying all active listeners to
 * the events.
 * Passes a copy of the given event objects to listeners.
 * @param e the event object to pass along to listeners
 * @param args the rest of the event objects
 */
template <typename EventType, typename... Args>
void trigger(EventType e, Args... args) {
    //std::thread t(Impl<EventType>::triggerImpl, e);
    //t.detach();
    Impl<EventType>::triggerImpl(e);
    trigger(args...);
}

/**
 * Register a listener to an event.
 * The listener will be called asynchronously in
 * the order it was added, and will be passed a copy of
 * the event object that the event was triggered with.
 * @tparam EventType the event to register the listener to
 * @param l the listener
 * @return a handle to the listener
 */
template <typename EventType>
ListenerHandle<EventType> registerListener(Listener<EventType> &&l) {
    Impl<EventType>::listeners.push_back(std::forward<Listener<EventType>>(l));
    return ListenerHandle<EventType>{--Impl<EventType>::listeners.end(), true};
}

/**
 * Register a listener to an event.
 * The listener will be called asynchronously in
 * the order it was added, and will be passed a copy of
 * the event object that the event was triggered with.
 * Serves as a utility function in situations where
 * a listener needs to be registered to an event based off
 * of an instance of the event.
 * @param l the listener
 * @return a handle to the listener
 */
template <typename EventType>
ListenerHandle<EventType> registerListener(EventType, Listener<EventType> &&l) {
    return registerListener<EventType>(std::forward<Listener<EventType>>(l));
}

/**
 * Deregister a listener from an event.
 * The listener corresponding to the given listener
 * handle will be deregistered from its event and will
 * no longer be called when the event is triggered.
 * @param lh the handle to the listener
 */
template <typename EventType>
void deregisterListener(ListenerHandle<EventType> &lh) {
    assert(lh.valid == true && "invalid listener ref");
    Impl<EventType>::listeners.erase(lh.iter);
    lh.valid = false;
}

void deregisterListeners();

template <typename EventType, typename... Args>
void deregisterListeners(ListenerHandle<EventType> &lh, Args... args) {
    deregisterListener(lh);
    deregisterListeners(args...);
}

template <typename EventType>
void waitFor() {
    std::unique_lock lock(Impl<EventType>::mut);
    Impl<EventType>::cond.wait(lock);
    lock.unlock();
}

} // namespace event

#endif
