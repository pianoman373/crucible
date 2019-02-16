#pragma once

#include <string>
#include <crucible/Texture.hpp>
#include "stb_truetype.h"

class Sprite;

class Font {
private:
    Texture texture;
    stbtt_bakedchar *cdata;

public:
    Font();

    ~Font();

    void loadFromFile(std::string path, float fontSize);

    const Texture &getTexture() const;

    std::vector<Sprite> getSprites(const std::string &message, vec4 color) const;
};