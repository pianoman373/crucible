#include "EditorContext.hpp"

#include <json.hpp>
#include <fstream>

EditorContext::EditorContext() {

}

void EditorContext::saveToConfig() {
    json j;

    j["projectPath"] = projectPath.toString();

    std::ofstream o("editorConfig.json");

    o << j;

    o.close();
}

void EditorContext::loadFromConfig() {
    json j;
    std::ifstream o("editorConfig.json");

    if(!o.fail()){
        o >> j;

        std::string p = j["projectPath"];
        projectPath = Path(p);
    }

    o.close();
}