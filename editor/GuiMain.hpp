#pragma once

#include <crucible/Model.hpp>
#include <crucible/Material.hpp>


class GuiMain {
private:
    Model *model;

    void renderMaterial(Material *mat);

public:
    int selected = -1;

    GuiMain(Model *model);

    void render();
};