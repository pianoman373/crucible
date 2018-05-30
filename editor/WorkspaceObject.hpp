#pragma once

#include <string>
#include <vector>

#include <crucible/Mesh.hpp>
#include <crucible/Material.hpp>

struct WorkspaceMesh {
    Mesh mesh;
    std::string name;
    int materialIndex;
};

struct WorkspaceMaterial {
    Material material;
    std::string name;
};

class WorkspaceObject {
private:

public:
    std::vector<WorkspaceMesh> meshes;
    std::vector<WorkspaceMaterial> materials;

    void import(std::string filename);

    void open(std::string filename);

    void render(int selection);

    void save(std::string path);

    void clear();
};
