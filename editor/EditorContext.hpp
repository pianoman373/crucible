#pragma once

#include <crucible/Model.hpp>
#include <crucible/Math.hpp>
#include <crucible/Scene.hpp>

#include <vector>
#include <string>

class EditorContext {
public:
    Scene scene;

    int selectedObject = -1;

    std::vector<Material*> materialCache;
    std::vector<Mesh*> meshCache;

    std::string projectPath = "./";

    void saveToConfig();

    void loadFromConfig();
};