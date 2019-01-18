#pragma once

#include "EditorContext.hpp"
#include <string>
#include <stack>

#include <crucible/Path.hpp>

class ProjectPanel {
private:
    EditorContext &context;

    Path path = Path(false);
    int selectedItem = -1;

    std::stack<Path> previousDirs;

public:

    ProjectPanel(EditorContext &context);

    void renderContents();
};