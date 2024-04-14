#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <list>
#include <thread>
#include <cassert>
#include <functional>

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

    static void triggerImpl(EventType e) {
        for (auto &&l : listeners) {
            std::invoke(l, e);
        }
    }
};
template <typename EventType>
typename Impl<EventType>::ListenerList Impl<EventType>::listeners = ListenerList();

/**
 * Trigger an event, notifying all active listeners to
 * this event.
 * Passes a copy of the given event object to listeners.
 * @param e the event object to pass along to listeners
 */
template <typename EventType>
void trigger(EventType e) {
    std::thread t(Impl<EventType>::triggerImpl, e);
    t.join();
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

} // namespace event

#endif
