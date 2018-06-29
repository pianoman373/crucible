#include <crucible/Path.hpp>

#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <stdio.h>
#endif

// https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string
template<typename Out>
static void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty())
            *(result++) = item;
    }
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
// ------------------------------------------------------------------------------------------------

namespace Path {



    void format(std::string &path) {
        std::replace(path.begin(), path.end(), '\\', '/');

        if (path[1] == ':') {
            path = path.substr(2, path.length());
        }
    }

    std::string getWorkingDirectory() {
        #ifdef _WIN32
            char cCurrentPath[FILENAME_MAX];
            GetCurrentDirectory(sizeof(cCurrentPath), cCurrentPath);

            std::string dir = std::string(cCurrentPath);
            format(dir);
            return dir;
        #else
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            return std::string(cwd);

        return 0;
        #endif
    }

    std::string getWorkingDirectory(const std::string path) {
        return path.substr(0, path.find_last_of("/")) + "/";
    }

    std::string getRelativePath(const std::string parent, const std::string child) {
        std::vector<std::string> parentList = split(parent, '/');
        std::vector<std::string> childList = split(child, '/');

        std::string finalPath = "";

        int childIndex = 0;

        for (int i = 0; i < parentList.size(); i++) {
            //std::cout << parentList[i] << std::endl;
            //std::cout << childList[i] << std::endl;
            if (!parentList[i].compare(childList[i])) {
                //std::cout << "equal" << std::endl;
                childIndex++;
            }
            else {
                //std::cout << "not equal" << std::endl;
                finalPath += "../";
            }
        }

        for (int i = childIndex; i < childList.size(); i++) {
            finalPath += childList[i];

            if (i + 1 < childList.size())
                finalPath += "/";
        }

        //finalPath += child.substr(childIndex, child.size());

        return finalPath;
    }

    std::string getFullPath(const std::string workingDirectory, const std::string path) {
        std::vector<std::string> workingDirectoryList = split(workingDirectory, '/');
        std::vector<std::string> pathList = split(path, '/');

        int upwardHops = 0;
        int pathIndex = 0;

        std::string finalString = "";
        std::string pathString = "";

        for (int i = 0; i < pathList.size(); i++) {
            if (!pathList[i].compare("..")) {
                upwardHops++;
            }
            else {
                pathString += "/" + pathList[i];
            }
        }

        if (upwardHops < workingDirectoryList.size()) {
            for (int i = 0; i < workingDirectoryList.size() - upwardHops; i++) {
                finalString += "/" + workingDirectoryList[i];
            }
        }

        finalString += pathString;

        if (workingDirectory[0] != '/') {
            finalString= finalString.substr(1, finalString.size());
        }

        return finalString;
    }

    std::string getFullPath(const std::string path) {
        return getFullPath(getWorkingDirectory(), path);
    }
}