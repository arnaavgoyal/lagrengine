#include <iostream>

#include "event.h"

template <typename EventType>
typename EventManager::Impl<EventType>::ListenerList EventManager::Impl<EventType>::listeners = ListenerList();

// class MyCustomEvent1 { };
// class MyCustomEvent2 { };

// int main() {
//     auto handle1 = EventManager::registerListener<MyCustomEvent1>(
//         [](MyCustomEvent1){ std::cout << "Listener1 saw event occurred!\n"; }
//     );
//     auto handle2 = EventManager::registerListener<MyCustomEvent2>(
//         [](MyCustomEvent2){ std::cout << "Listener2 saw event occurred!\n"; }
//     );

//     MyCustomEvent1 e1;
//     MyCustomEvent2 e2;

//     EventManager::trigger(e2);
//     EventManager::trigger(e1);
//     EventManager::trigger(e1);
//     EventManager::deregisterListener(handle1);
//     EventManager::trigger(e1);
//     EventManager::trigger(e2);
//     EventManager::deregisterListener(handle2);
//     EventManager::trigger(e2);
// }
