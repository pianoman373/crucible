#include <crucible/Renderer2D.hpp>
#include <crucible/InternalShaders.hpp>
#include <crucible/Shader.hpp>
#include <crucible/Mesh.hpp>
#include <crucible/Primitives.hpp>

#include <glad/glad.h>

static std::vector<RenderCallSprite> renderQueueSprite;
static Shader spriteShader;
static Mesh spriteMesh;

namespace Renderer2D {
    void init() {
        spriteShader.load(InternalShaders::sprite_vsh, InternalShaders::sprite_fsh);
        Primitives::sprite(spriteMesh);
    }

    void renderSprite(Texture tex, vec2 pos, vec2 dimensions, vec4 uv) {
        RenderCallSprite call;
        call.tex = tex;
        call.pos = pos;
        call.dimensions = dimensions;
        call.uv = uv;

        renderQueueSprite.push_back(call);
    }

    void flush(Camera cam) {
        // render any sprites
        // ------------------
        glDepthMask(GL_FALSE);
        for (unsigned int i = 0; i < renderQueueSprite.size(); i++) {
            RenderCallSprite call = renderQueueSprite[i];

            spriteShader.bind();

            call.tex.bind(0);

            spriteShader.uniformVec4("uvOffsets", call.uv);

            mat4 model;
            model = translate(model, vec3(call.pos.x, call.pos.y, 0.0f));
            model = scale(model, vec3(call.dimensions.x, call.dimensions.y, 0.0f));


            spriteShader.uniformMat4("model", model);
            spriteShader.uniformMat4("view", cam.getView());
            spriteShader.uniformMat4("projection", cam.getProjection());
            spriteShader.uniformVec3("cameraPos", cam.getPosition());

            spriteMesh.render();
        }
        glDepthMask(GL_TRUE);

        renderQueueSprite.clear();
    }
}