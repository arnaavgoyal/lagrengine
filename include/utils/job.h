#ifndef UTILS_JOB_H
#define UTILS_JOB_H

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <concepts>
#include <type_traits>
#include <functional>
#include <syncstream>

#include "utils/tsq.h"
#include "utils/thread.h"
#include "utils/log.h"
#include "engine.h"

struct JobManager {

    struct Job;

    struct Edge {
        Job *dependent;
        Job *dependency;
    };

    struct Job {
        unsigned id;
        std::string name;
        std::function<void (void *)> entry;
        void *arg;
        std::vector<Edge *> dependencies;
        std::vector<Edge *> dependents;
    };

    bool compiled;
    std::list<Edge> edges;
    std::map<std::string, Job> jobs;
    Job *root;
    ThreadPool threads;
    unsigned *dependency_matrix;
    std::mutex dep_mat_sync;

    static void rootDummyFuncImpl(void *) {
        //std::osyncstream(std::cerr) << "Running __root\n";
    }

    JobManager() :
        compiled(false),
        threads(1),
        dependency_matrix(nullptr) {
        
        root = &jobs["__root"];
        root->name = "__root";
        root->entry = rootDummyFuncImpl;
        root->arg = nullptr;
    }

    ~JobManager() {
        //std::osyncstream(std::cerr) << "Destructing JobManager@" << this << "\n";
        if (compiled) {
            delete[] dependency_matrix;
        }
    }

    Job *findJob(std::string name) {
        return &jobs.at(name);
    }

    Job *registerJob(std::string name, std::function<void (void *)> ef, void *arg) {
        assert(!compiled);
        Job &j = jobs[name];
        j.name = name;
        j.entry = ef;
        j.arg = arg;
        registerDependency(&j, root);
        return &j;
    }

    Edge *registerDependency(Job *dependent, Job *dependency) {
        assert(!compiled);
        Edge *edge = &edges.emplace_back(Edge{dependent, dependency});
        dependent->dependencies.push_back(edge);
        dependency->dependents.push_back(edge);
        return edge;
    }

    std::ostream &dumpGraph(std::ostream &os) const {
        os << "digraph {\n";
        for (auto &edge : edges) {
            os << "    " << edge.dependent->name << " -> " << edge.dependency->name;
            if (edge.dependency == root) {
                os << " [style=\"dashed\"]";
            }
            os << "\n";
        }
        os << "}\n";
        return os;
    }

    friend std::ostream &operator<<(std::ostream &os, JobManager const &manager) {
        return manager.dumpGraph(os);
    }

    void compile() {
        assert(!compiled);
        compiled = true;

        // generate dependency matrix
        dependency_matrix = new unsigned[jobs.size()];
        unsigned id_gen = 0;
        for (auto &[name, job] : jobs) {
            job.id = id_gen;
            dependency_matrix[id_gen] = job.dependencies.size();
            //std::osyncstream(std::cerr) << std::format("Job '{}': id={}, dm[id]={}\n", name, id_gen, dependency_matrix[id_gen]);
            id_gen++;
        }

        assert(dependency_matrix[root->id] == 0);
    }

    static void jobRunner(JobManager &jm, Job &job) {

        //std::osyncstream(std::cerr) << std::format("Starting job '{}'...\n", job.name);

        // run job
        std::invoke(job.entry, job.arg);

        //std::osyncstream(std::cerr) << std::format("Finished job '{}'...\n", job.name);
        
        //  update all dependents
        for (Edge *edge : job.dependents) {
            //std::osyncstream(std::cerr) << std::format("{} -- Checking dependent '{}'...\n", job.name, edge->dependent->name);

            // get entry from dependency matrix
            unsigned *dep_entry = jm.dependency_matrix + edge->dependent->id;

            jm.dep_mat_sync.lock();

            // >> CRITICAL SECTION

            //std::osyncstream(std::cerr) << std::format("{} -- CRITICAL SECTION start...\n", job.name);

            assert(*dep_entry > 0);

            // update dependency counter
            *dep_entry -= 1;

            // if there are no dependencies left
            if (*dep_entry == 0) {
                
                // reset this job's dependency counter for next iteration
                *dep_entry = edge->dependent->dependencies.size();

                // run it
                jm.threads.run(jobRunner, jm, *edge->dependent);
            }

            // << CRITICAL SECTION

            //std::osyncstream(std::cerr) << std::format("{} -- CRITICAL SECTION end...\n", job.name);

            jm.dep_mat_sync.unlock();
        }
    }

    void runIteration() {
        threads.run(jobRunner, *this, *root);
    }

};



#endif
