#pragma once

#include <string>
#include <vector>

#include <crucible/Mesh.hpp>
#include <crucible/Material.hpp>
#include <crucible/Model.hpp>

class WorkspaceObject {
private:

public:
    Model model;

    void import(std::string filename);

    void open(std::string filename);

    void render(int selection);

    void save(std::string path);

    void clear();
};
