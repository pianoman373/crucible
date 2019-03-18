#pragma once

#include <crucible/Model.hpp>
#include <crucible/Math.hpp>
#include <crucible/Scene.hpp>
#include <crucible/Path.hpp>

#include <vector>
#include <string>


class EditorContext {
public:

    std::vector<Material*> materialCache;
    std::vector<Mesh*> meshCache;

    Path projectPath = Path(false);

    EditorContext();

    void saveToConfig();

    void loadFromConfig();

    void renderContents();
};