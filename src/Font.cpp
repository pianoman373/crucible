#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include <crucible/Font.hpp>

#include <crucible/GuiRenderer.hpp>


#include <fstream>
#include <glad/glad.h>

Font::Font() {
    cdata = new stbtt_bakedchar[96];
}

Font::~Font() {
    delete[] cdata;
}

#define RESOLUTION 1024

void Font::loadFromFile(std::string path, float fontSize) {
    unsigned char *temp_bitmap = new unsigned char[RESOLUTION*RESOLUTION];
    unsigned char *bitmap = new unsigned char[RESOLUTION*RESOLUTION*4];



    FILE* fontFile = fopen(path.c_str(), "rb");
    fseek(fontFile, 0, SEEK_END);
    long size = ftell(fontFile); /* how long is the file ? */
    fseek(fontFile, 0, SEEK_SET); /* reset */

    unsigned char *ttf_buffer = new unsigned char[size];

    fread(ttf_buffer, size, 1, fontFile);
    fclose(fontFile);

    stbtt_BakeFontBitmap(ttf_buffer,0, fontSize, temp_bitmap,RESOLUTION,RESOLUTION, 32,96, cdata); // no guarantee this fits!

    for (int i = 0; i < RESOLUTION*RESOLUTION; i++) {
        bitmap[i*4] = 255;
        bitmap[i*4 + 1] = 255;
        bitmap[i*4 + 2] = 255;
        bitmap[i*4 + 3] = temp_bitmap[i];
    }

    texture.load(bitmap, RESOLUTION, RESOLUTION, false);

    delete[] ttf_buffer;
    delete[] temp_bitmap;
    delete[] bitmap;
}

const Texture& Font::getTexture() const {
    return texture;
}

std::vector<Sprite> Font::getSprites(const std::string &message, vec4 color) const {
    const char *text = message.c_str();
    std::vector<Sprite> ret;

    float x = 0.0f;
    float y = 0.0f;

    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, RESOLUTION,RESOLUTION, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9

            vec2 position = vec2((q.x0 + q.x1) / 2.0f, (q.y0 + q.y1) / 2.0f);
            vec2 size = vec2(q.x1 - q.x0, q.y1 - q.y0);
            vec4 uvs = vec4(q.s0, q.t0, q.s1, q.t1);

            ret.push_back({position, size, uvs, color, texture});
        }
        ++text;
    }

    return ret;
}