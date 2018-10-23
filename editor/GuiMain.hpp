#pragma once

#include <crucible/Model.hpp>
#include <crucible/Material.hpp>


class GuiMain {
private:
    Model *model;

    void renderMaterial(Material *mat);

    void renderToolbar();

    void renderSceneWindow();

    void renderMaterialWindow();

public:
    int selected = -1;

    GuiMain(Model *model);

    void render();
};