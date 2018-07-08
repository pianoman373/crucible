#include <crucible/Math.hpp>
#include <crucible/Texture.hpp>
#include <crucible/Camera.hpp>

#include <vector>


struct RenderCallSprite {
    Texture tex;
    vec2 pos;
    vec2 dimensions;
    vec4 uv;
};

namespace Renderer2D {
    void init();

    void renderSprite(Texture tex, vec2 pos, vec2 dimensions, vec4 uv);

    void flush(Camera cam);
}