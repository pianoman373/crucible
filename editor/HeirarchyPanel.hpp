#pragma once


#include <crucible/Camera.hpp>
#include <crucible/GameObject.hpp>

class PrefabView;

class HeirarchyPanel {
private:
    PrefabView &view;

    int selection = -1;

    void renderNode(GameObject &obj, int &i);

public:

    HeirarchyPanel(PrefabView &view);

    void renderContents();
};