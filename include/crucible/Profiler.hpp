#pragma once

#include <string>
#include <vector>

namespace Profiler {
    void begin(std::string name);

    void end();

    std::vector<std::pair<std::string, double>> getValues();
}