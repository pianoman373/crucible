#pragma once


#include <string>
#include <stack>

#include <crucible/Path.hpp>

class PrefabView;

class ProjectPanel {
private:
    PrefabView &view;

    Path path = Path(false);
    int selectedItem = -1;

    std::stack<Path> previousDirs;

public:

    ProjectPanel(PrefabView &view);

    void renderContents();
};