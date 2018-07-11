#pragma once

#include <crucible/Mesh.hpp>
#include <crucible/Material.hpp>

#include <vector>
#include <string>

struct ModelNode {
    Mesh mesh;
    int materialIndex;
    std::string name;
};

class Model {
public:
    std::vector<Material> materials;
    std::vector<ModelNode> nodes;

    void addSubmesh(Mesh mesh, Material material, std::string names="Untitled Submesh");

    void importFile(std::string filename, bool loadTextures=true);

    void openFile(std::string filename);

    void fromJson(json j, std::string workingDirectory);

    json toJson(std::string workingDirectory);

    void clear();
};