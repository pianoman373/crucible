#include "WorkspaceObject.hpp"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <crucible/Renderer.hpp>
#include <crucible/Path.hpp>
#include <crucible/Resources.hpp>
#include <json.hpp>
#include <fstream>

void WorkspaceObject::import(std::string filename) {
    model.importFile(filename, true);
}

void WorkspaceObject::open(std::string filename) {
    Path::format(filename);

    json j;
    std::ifstream o(filename);
    o >> j;

    model.fromJson(j, Path::getWorkingDirectory(filename));
}

void WorkspaceObject::render(int selection) {
    Renderer::render(&model, Transform(vec3(), quaternion(), vec3(1.0f)), AABB());
}

void WorkspaceObject::save(std::string path) {
    std::string originalPath = path;
    Path::format(path);

    json j = model.toJson(Path::getWorkingDirectory(path));

    std::ofstream o(originalPath);
    o << std::setw(4) << j << std::endl;
}

void WorkspaceObject::clear() {
    model.clear();
}