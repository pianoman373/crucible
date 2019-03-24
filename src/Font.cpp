#include <crucible/Font.hpp>


#include <fstream>
#include <glad/glad.h>

Font::Font() {
}

Font::~Font() {
}

void Font::loadFromFile(const std::string &path, float fontSize) {
    this->fontSize = fontSize;

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    for (unsigned char c = 0; c < 255; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        Texture tex;
        tex.load(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, false, true);

        // Now store character for later use
        Character character = {
                tex,
                vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
        };
        this->characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

vec2i Font::getTextSize(const std::string &text) const {
    vec2i size = vec2i(0.0f, fontSize);

   for (int i = 0; i < text.size(); i++) {
        const Character &ch = characters.at(text[i]);


        size.x += ch.bearing.x;

        if (i != text.size() - 1) {
            size.x += (ch.advance >> 6);
        }
    }

    return size;
}