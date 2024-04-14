#include <iostream>

#include "utils/event.h"

class MyCustomEvent1 { };
class MyCustomEvent2 { };

int mainImpl(int argc, char **argv) {
    auto handle1 = event::registerListener<MyCustomEvent1>(
        [](MyCustomEvent1){ std::cout << "Listener1 saw event occurred!\n"; }
    );
    auto handle2 = event::registerListener<MyCustomEvent2>(
        [](MyCustomEvent2){ std::cout << "Listener2 saw event occurred!\n"; }
    );

    MyCustomEvent1 e1;
    MyCustomEvent2 e2;

    event::trigger(e2);
    event::trigger(e1);
    event::trigger(e1);
    event::deregisterListener(handle1);
    event::trigger(e1);
    event::trigger(e2);
    event::deregisterListener(handle2);
    event::trigger(e2);
    return 0;
}

// Comment or uncomment this accordingly:
//   If you want console application, leave this uncommented
//   If you want windowed application, comment this out
//int main(int argc, char **argv) { return mainImpl(argc, argv); }
