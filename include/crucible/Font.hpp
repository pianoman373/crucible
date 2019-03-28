#pragma once

#include <string>
#include <crucible/Texture.hpp>

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    Texture tex;
    vec2i size;       // Size of glyph
    vec2i bearing;    // Offset from baseline to left/top of glyph
    int advance;    // Offset to advance to next glyph
};

class Font {
private:
    float fontSize;

public:
    float descender;

    std::map<unsigned char, Character> characters;



    Font();

    ~Font();

    void loadFromFile(const std::string &path, float fontSize);

    vec2i getTextSize(const std::string &text) const;
};