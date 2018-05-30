#pragma once

#include <string>

namespace Path {
    void format(std::string &path);

    std::string getWorkingDirectory(const std::string path);

    std::string getWorkingDirectory();

    std::string getRelativePath(const std::string parent, const std::string child);

    std::string getFullPath(const std::string workingDirectory, const std::string path);

    std::string getFullPath(const std::string path);
}