#pragma once

#include "EditorContext.hpp"

class Panel {
private:
    EditorContext &context;

public:
    Panel(EditorContext &context);

    virtual void renderContents();
};