#include <crucible/Profiler.hpp>

#include <ctime>
#include <iostream>
#include <vector>
#include <stack>
#include <map>

namespace Profiler {
    struct entry {
        clock_t start;
        std::string name;

    };

    static std::stack<std::pair<clock_t, std::string>> stack;
    static std::map<std::string, double> times;

    void begin(std::string name) {
        stack.push({clock(), name});
    }

    void end() {
        clock_t tstart = stack.top().first;
        std::string name = stack.top().second;
        stack.pop();

        double timeElapsed = (((double)(clock() - tstart)) / CLOCKS_PER_SEC) * 1000.0;

        times[name] = timeElapsed;
    }

    std::vector<std::pair<std::string, double>> getValues() {
        std::vector<std::pair<std::string, double>> values;

        for (auto &i : times) {
            values.push_back(i);
        }

        return values;
    };
}