#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <windows.h>
#include <format>
#include <syncstream>

#include "engine.h"
#include "utils/event.h"
#include "utils/job.h"
#include "utils/log.h"
#include "utils/tsq.h"
#include "utils/thread.h"

struct SimulationStartEvent { };
struct SimulationEndEvent { };
struct RenderStartEvent { };
struct RenderEndEvent { };

void engineMain() {

    // ...

    while (true) {

        // ...

        SimulationStartEvent sse;
        //event::trigger(sse);

        // ...

        SimulationEndEvent see;
        //event::trigger(see);

        // ...

        RenderStartEvent rse;
        //event::trigger(rse);

        // ...

        RenderEndEvent ree;
        //event::trigger(ree);

        // ...

        std::this_thread::sleep_for(std::chrono::seconds(20));

    }
}

void engineInit() {

    event::registerListener<SimulationEndEvent>(
        [](SimulationEndEvent){
            std::osyncstream(std::cerr) << "Sim ended! w/ cout\n";
            std::osyncstream(std::cerr) << "Sim ended! w/ cerr\n";
        }
    );

    event::registerListener<WindowCloseRequestedEvent>(
        [](WindowCloseRequestedEvent e){
            std::osyncstream(std::cerr) << "Window close requested!\n";
        }
    );

    event::registerListener<WindowDestroyStartEvent>(
        [](WindowDestroyStartEvent e){
            std::osyncstream(std::cerr) << "Window destroy start...\n";
        }
    );

    event::registerListener<WindowDestroyEndEvent>(
        [](WindowDestroyEndEvent e){
            std::osyncstream(std::cerr) << "Window destroy end!\n";
        }
    );

    bool done = false;

    JobManager jm;
    auto simj = jm.registerJob("Simulation", [](void *){ std::osyncstream(std::cerr) << "Completing Simulation job...\n"; }, nullptr);
    auto phyj = jm.registerJob("Physics", [](void *){ std::osyncstream(std::cerr) << "Completing Physics job...\n"; }, nullptr);
    auto occj = jm.registerJob("Occlusion", [](void *){ std::osyncstream(std::cerr) << "Completing Occlusion job...\n"; }, nullptr);
    auto renj = jm.registerJob("Rendering", [](void *){ std::osyncstream(std::cerr) << "Completing Rendering job...\n"; }, nullptr);
    auto winj = jm.registerJob("WindowPainting", [&done](void *){ done = true; std::osyncstream(std::cerr) << "Completing Window Painting job...\n"; }, nullptr);

    jm.registerDependency(occj, simj);
    jm.registerDependency(occj, phyj);
    jm.registerDependency(renj, simj);
    jm.registerDependency(renj, occj);
    jm.registerDependency(winj, renj);

    std::fstream out("jobgraph.dot", std::fstream::out);
    out << jm;
    out.close();

    jm.compile();
    jm.runIteration();

    // ...

    engineMain();

    // ...

}
