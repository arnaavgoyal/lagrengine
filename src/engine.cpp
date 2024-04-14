#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>

#include "engine.h"
#include "utils/event.h"

struct SimulationStartEvent { };
struct SimulationEndEvent { };
struct RenderStartEvent { };
struct RenderEndEvent { };

void engineMain() {

    // ...

    SimulationStartEvent sse;
    event::trigger(sse);

    // ...

    SimulationEndEvent see;
    event::trigger(see);

    // ...

    RenderStartEvent rse;
    event::trigger(rse);

    // ...

    RenderEndEvent ree;
    event::trigger(ree);

    // ...

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

void engineInit() {

    event::registerListener<SimulationEndEvent>(
        [](SimulationEndEvent){
            std::cout << "Sim ended! w/ cout\n";
            std::cerr << "Sim ended! w/ cerr\n";
        }
    );

    // ...

    engineMain();

    // ...
}
