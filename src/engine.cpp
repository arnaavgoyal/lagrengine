#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <windows.h>

#include "engine.h"
#include "utils/event.h"
#include "utils/job.h"

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

    event::registerListener<SimulationEndEvent>(
        [](SimulationEndEvent){
            std::cout << "Sim ended! w/ cout\n";
            std::cerr << "Sim ended! w/ cerr\n";
        }
    );

    JobManager jm;
    auto simj = jm.registerJob("Simulation");
    auto occlusionj = jm.registerJob("Occlusion");
    jm.registerDependency(occlusionj, simj);
    auto renderj = jm.registerJob("Rendering");
    jm.registerDependency(renderj, simj);
    jm.registerDependency(renderj, occlusionj);

    std::fstream out("jobgraph.dot", std::fstream::out);
    out << jm;
    out.close();

    // ...

    engineMain();

    // ...
    
}
