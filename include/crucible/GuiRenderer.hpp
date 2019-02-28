#pragma once

#include <crucible/Math.hpp>
#include <crucible/Texture.hpp>
#include <crucible/Mesh.hpp>
#include <crucible/Font.hpp>

struct Sprite {
    vec2 position;
    vec2 size;
    vec4 uvs;
    vec4 color;
    const Texture &tex;
};

class GuiRenderer {
private:


public:
    static void renderSprite(vec2 position, vec2 size, vec4 uvs, vec4 color, const Texture &tex);

    static void renderSprite(vec2 position, vec2 size, vec4 color);

    static void renderText(vec2 position, const std::string &text, const Font &font, vec4 color);
};