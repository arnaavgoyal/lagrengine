#ifndef UTILS_JOB_H
#define UTILS_JOB_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

struct JobManager {

    struct Job;

    struct Edge {
        Job *dependent;
        Job *dependency;
    };

    struct Job {
        std::string name;
        std::vector<Edge *> dependencies;
        std::vector<Edge *> dependents;
    };

    std::vector<Edge> edges;
    std::map<std::string, Job> jobs;

    Job *findJob(std::string name) {
        return &jobs.at(name);
    }

    Job *registerJob(std::string name) {
        Job &j = jobs[name];
        j.name = name;
        return &j;
    }

    Edge *registerDependency(Job *dependent, Job *dependency) {
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
};

#endif
