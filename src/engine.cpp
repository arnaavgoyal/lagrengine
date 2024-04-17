#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <syncstream>
#include <thread>
#include <windows.h>

#include "engine.h"
#include "utils/event.h"
#include "utils/job.h"
#include "utils/log.h"
#include "utils/thread.h"
#include "utils/tsq.h"

struct SimulationStartEvent { };
struct SimulationEndEvent { };
struct RenderStartEvent { };
struct RenderEndEvent { };

void engineMain() {

    bool done;

    JobManager jm;
    auto simj = jm.registerJob("Simulation", [](void *){ std::osyncstream(std::cerr) << "Completing Simulation job...\n"; }, nullptr);
    auto phyj = jm.registerJob("Physics", [](void *){ std::osyncstream(std::cerr) << "Completing Physics job...\n"; }, nullptr);
    auto occj = jm.registerJob("Occlusion", [](void *){ std::osyncstream(std::cerr) << "Completing Occlusion job...\n"; }, nullptr);
    auto renj = jm.registerJob("Rendering", [](void *){ std::osyncstream(std::cerr) << "Completing Rendering job...\n"; }, nullptr);
    auto winj = jm.registerJob("WindowPainting", [&done](void *){ std::osyncstream(std::cerr) << "Completing Window Painting job...\n"; done = true; }, nullptr);

    jm.registerDependency(occj, simj);
    jm.registerDependency(occj, phyj);
    jm.registerDependency(renj, simj);
    jm.registerDependency(renj, occj);
    jm.registerDependency(winj, renj);

    std::fstream out("jobgraph.dot", std::fstream::out);
    out << jm;
    out.close();

    jm.compile();

    while (true) {

        done = false;
        jm.runIteration();
        while (!done) { }
        std::osyncstream(std::cerr) << "All jobs done!\n";

        // ...

        std::this_thread::sleep_for(std::chrono::seconds(10));

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

    // ...

    engineMain();

    // ...

}
