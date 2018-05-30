#pragma once

#include <crucible/Mesh.hpp>
#include <crucible/Material.hpp>

#include <vector>
#include <string>

class Model {
public:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<std::string> names;

    void addSubmesh(Mesh mesh, Material material, std::string names="Untitled Submesh");

    void loadFile(std::string filename, bool loadTextures=true);

    void clear();
};