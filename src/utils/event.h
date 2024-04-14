#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <list>
#include <thread>
#include <cassert>
#include <functional>
#include <type_traits>

class EventManager {
private:

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
        static void trigger(EventType e) {
            std::thread t(triggerImpl, e);
            t.join();
        }
        static ListenerHandle<EventType> registerListener(Listener<EventType> &&l) {
            listeners.push_back(l);
            return ListenerHandle<EventType>{--listeners.end(), true};
        }
        static void deregisterListener(ListenerHandle<EventType> &lh) {
            assert(lh.valid == true && "invalid listener ref");
            listeners.erase(lh.iter);
            lh.valid = false;
        }
    };

public:

    template <typename EventType>
    static void trigger(EventType e) {
        Impl<EventType>::trigger(std::forward<EventType>(e));
    }

    template <typename EventType>
    static ListenerHandle<EventType> registerListener(Listener<EventType> &&l) {
        return Impl<EventType>::registerListener(std::forward<Listener<EventType>>(l));
    }
    template <typename EventType>
    static ListenerHandle<EventType> registerListener(EventType, Listener<EventType> &&l) {
        return registerListenerFor<EventType>(std::forward<Listener<EventType>>(l));
    }

    template <typename EventType>
    static void deregisterListener(ListenerHandle<EventType> &lh) {
        Impl<EventType>::deregisterListener(lh);
    }
    template <typename EventType>
    static void deregisterListener(EventType, ListenerHandle<EventType> &lh) {
        Impl<EventType>::deregisterListener(lh);
    }
};

#endif
