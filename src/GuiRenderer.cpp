#include <crucible/GuiRenderer.hpp>
#include <crucible/Primitives.hpp>
#include <crucible/Resource.h>
#include <crucible/Renderer.hpp>
#include <crucible/Window.hpp>

void GuiRenderer::renderSprite(vec2 position, vec2 size, vec4 uvs, vec4 color, const Texture &tex) {
    Renderer::spriteShader.bind();
    vec2i res = Window::getWindowSize();

    mat4 model;

    model = translate(model, vec3(position.x, position.y, 0.0f));
    model = scale(model, vec3(size.x, size.y, 0.0f));

    mat4 view;

    mat4 projection = orthographic(0.0f, res.x, res.y, 0.0f, -1.0f, 1.0f);

    Renderer::spriteShader.uniformMat4("model", model);
    Renderer::spriteShader.uniformMat4("view", view);
    Renderer::spriteShader.uniformMat4("projection", projection);
    Renderer::spriteShader.uniformVec4("uvOffsets", uvs);
    Renderer::spriteShader.uniformVec4("color", color);
    Renderer::spriteShader.uniformFloat("textureStrength", 1.0f);
    tex.bind();

    Renderer::spriteMesh.render();

}

void GuiRenderer::renderSprite(vec2 position, vec2 size, vec4 color) {
    Renderer::spriteShader.bind();
    vec2i res = Window::getWindowSize();

    mat4 model;

    model = translate(model, vec3(position.x, position.y, 0.0f));
    model = scale(model, vec3(size.x, size.y, 0.0f));

    mat4 view;

    mat4 projection = orthographic(0.0f, res.x, res.y, 0.0f, -1.0f, 1.0f);

    Renderer::spriteShader.uniformMat4("model", model);
    Renderer::spriteShader.uniformMat4("view", view);
    Renderer::spriteShader.uniformMat4("projection", projection);
    Renderer::spriteShader.uniformVec4("color", color);
    Renderer::spriteShader.uniformFloat("textureStrength", 0.0f);
    Texture::bindNull();


    Renderer::spriteMesh.render();

}

void GuiRenderer::renderText(vec2 position, const std::string &text, const Font &font, vec4 color) {
    std::vector<Sprite> sprites = font.getSprites(text, color);

    for (int i = 0; i < sprites.size(); i++) {
        Sprite &s = sprites[i];

        renderSprite(s.position + position, s.size, s.uvs, s.color, s.tex);
    }
}