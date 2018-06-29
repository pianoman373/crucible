#pragma once

#include "WorkspaceObject.hpp"


class GuiMain {
private:
    WorkspaceObject *workspace;

    void renderMaterial(Material *mat);

public:
    int selected = -1;

    GuiMain(WorkspaceObject *workspace);

    void render();
};