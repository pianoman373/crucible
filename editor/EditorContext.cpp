#include "EditorContext.hpp"

#include <json.hpp>
#include <fstream>

void EditorContext::saveToConfig() {
    json j;

    j["projectPath"] = projectPath;

    std::ofstream o("editorConfig.json");

    o << j;

    o.close();
}

void EditorContext::loadFromConfig() {
    json j;
    std::ifstream o("editorConfig.json");

    if(!o.fail()){
        o >> j;

        projectPath = j["projectPath"];
    }

    o.close();
}