#pragma once

#include <crucible/Mesh.hpp>
#include <crucible/Model.hpp>
#include <crucible/Material.hpp>
#include <string>

class MeshImporter {
public:
    static Model loadFile(std::string filename, Material mat);
};