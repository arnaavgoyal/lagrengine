#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <windows.h>
#include <format>

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
}

void engineInit() {

    Log log(std::cerr);

    event::registerListener<SimulationEndEvent>(
        [&log](SimulationEndEvent){
            log << "Sim ended! w/ cout\n";
            log << "Sim ended! w/ cerr\n";
        }
    );

    event::registerListener<WindowCloseRequestedEvent>(
        [&log](WindowCloseRequestedEvent e){
            log << "Window close requested!\n";
        }
    );

    event::registerListener<WindowDestroyStartEvent>(
        [&log](WindowDestroyStartEvent e){
            log << "Window destroy start...\n";
        }
    );

    event::registerListener<WindowDestroyEndEvent>(
        [&log](WindowDestroyEndEvent e){
            log << "Window destroy end!\n";
        }
    );

    // JobManager jm;
    // auto simj = jm.registerJob("Simulation", nullptr, nullptr);
    // auto occlusionj = jm.registerJob("Occlusion", nullptr, nullptr);
    // jm.registerDependency(occlusionj, simj);
    // auto renderj = jm.registerJob("Rendering", nullptr, nullptr);
    // jm.registerDependency(renderj, simj);
    // jm.registerDependency(renderj, occlusionj);

    // std::fstream out("jobgraph.dot", std::fstream::out);
    // out << jm;
    // out.close();

    auto task = [&log](unsigned id){ log << std::format("Thread #{}\n", id); };

    ThreadPool tp(2);
    tp.run(task, 1)
        .run(task, 2)
        .run(task, 3)
        .run(task, 4)
        .init()
        .run(task, 5)
        .run(task, 6)
        .run(task, 7)
        .killAll();

    // ...

    engineMain();

    // ...

}
