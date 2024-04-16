#ifndef UTILS_JOB_H
#define UTILS_JOB_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <concepts>
#include <type_traits>
#include <functional>

#include "utils/tsq.h"

struct JobManager {

    using EntryFunc = void (*)(void *);
    struct Job;

    struct Edge {
        Job *dependent;
        Job *dependency;
    };

    struct Job {
        std::string name;
        EntryFunc entry;
        void *arg;
        std::vector<Edge *> dependencies;
        std::vector<Edge *> dependents;
    };

    bool locked;
    Job root;
    std::vector<Edge> edges;
    std::map<std::string, Job> jobs;

    static void rootDummyFuncImpl(void *) { }

    JobManager() : locked(false), root{"__root", rootDummyFuncImpl, nullptr} { }

    Job *findJob(std::string name) {
        return &jobs.at(name);
    }

    Job *registerJob(std::string name, EntryFunc ef, void *arg) {
        assert(!locked);
        Job &j = jobs[name];
        j.name = name;
        j.entry = ef;
        j.arg = arg;
        registerDependency(&j, &root);
        return &j;
    }

    Edge *registerDependency(Job *dependent, Job *dependency) {
        assert(!locked);
        Edge *edge = &edges.emplace_back(Edge{dependent, dependency});
        dependent->dependencies.push_back(edge);
        dependency->dependents.push_back(edge);
        return edge;
    }

    std::ostream &dumpGraph(std::ostream &os) const {
        os << "digraph {\n";
        for (auto &edge : edges) {
            os << "    " << edge.dependent->name << " -> " << edge.dependency->name << "\n";
        }
        os << "}\n";
        return os;
    }

    friend std::ostream &operator<<(std::ostream &os, JobManager const &manager) {
        return manager.dumpGraph(os);
    }

    void compile() {
        assert(!locked);
        locked = true;

    }

    static void jobRunnerFunc(Job &job) {
        
        // anything before running job
        // ...

        // run job
        std::invoke(job.entry, job.arg);

        // anything after job is finished
        // ...
    }

    void runJobImpl(Job &job) {

        // spin up a runner thread
        std::thread t(jobRunnerFunc, std::ref(job));

        // 
    }

    void runIteration() {
        runJobImpl(root);
    }

};



#endif
