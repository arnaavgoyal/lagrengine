#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <syncstream>
#include <thread>
#include <windows.h>

#include "engine.h"
#include "threading/job.h"
#include "threading/thread.h"
#include "utils/event.h"
#include "utils/log.h"
#include "utils/tsq.h"

struct SimulationStartEvent { };
struct SimulationEndEvent { };
struct RenderStartEvent { };
struct RenderEndEvent { };

void engineMain() {

    bool done;

    JobManager jm;
    auto job_func = [](std::string name, void *){ std::osyncstream(std::cerr) << logNow() << "Doing Job '" << name << "'\n"; };

    auto inij = jm.registerJob("init", std::bind(job_func, "init", std::placeholders::_1), nullptr);
    auto simj = jm.registerJob("simulation", std::bind(job_func, "simulation", std::placeholders::_1), nullptr);
    auto pyj1 = jm.registerJob("physics1", std::bind(job_func, "physics1", std::placeholders::_1), nullptr);
    auto pyj2 = jm.registerJob("physics2", std::bind(job_func, "physics2", std::placeholders::_1), nullptr);
    auto pyj3 = jm.registerJob("physics3", std::bind(job_func, "physics3", std::placeholders::_1), nullptr);
    auto pyj4 = jm.registerJob("physics4", std::bind(job_func, "physics4", std::placeholders::_1), nullptr);
    auto pyjf = jm.registerJob("physicsF", std::bind(job_func, "physicsF", std::placeholders::_1), nullptr);
    auto occj = jm.registerJob("occlusion", std::bind(job_func, "occlusion", std::placeholders::_1), nullptr);
    auto renj = jm.registerJob("render", std::bind(job_func, "render", std::placeholders::_1), nullptr);
    auto winj = jm.registerJob("window", std::bind(job_func, "window", std::placeholders::_1), nullptr);
    auto resj = jm.registerJob("reset", std::bind(job_func, "reset", std::placeholders::_1), nullptr);
    auto finj = jm.registerJob("finish", [&done](void *){ std::osyncstream(std::cerr) << logNow() << "Doing Job 'finish'\n"; done = true; }, nullptr);

    jm.registerDependencies(inij, jm.graphRoot());
    jm.registerDependencies(simj, inij);
    jm.registerDependencies(pyj1, inij);
    jm.registerDependencies(pyj2, inij);
    jm.registerDependencies(pyj3, inij);
    jm.registerDependencies(pyj4, inij);
    jm.registerDependencies(pyjf, pyj1, pyj2, pyj3, pyj4);
    jm.registerDependencies(occj, simj, pyjf);
    jm.registerDependencies(renj, simj, occj);
    jm.registerDependencies(winj, renj);
    jm.registerDependencies(resj, renj, occj);
    jm.registerDependencies(finj, resj, winj);

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
