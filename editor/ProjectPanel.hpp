#pragma once

#include "EditorContext.hpp"
#include <string>
#include <stack>

class ProjectPanel {
private:
    EditorContext &context;

    std::string path = "";
    int selectedItem = -1;

    std::stack<std::string> previousDirs;

public:

    ProjectPanel(EditorContext &context);

    void renderContents();
};