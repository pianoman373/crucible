#pragma once

#include <crucible/Path.hpp>
#include <crucible/Shader.hpp>
#include <crucible/Texture.hpp>
#include <crucible/AssimpFile.hpp>
#include <crucible/Material.hpp>

namespace Resources {
    extern Mesh cubemapMesh;
    extern Mesh framebufferMesh;
    extern Mesh spriteMesh;

    extern Shader standardShader;
    extern Shader eq2cubeShader;
    extern Shader cubemapShader;
    extern Shader irradianceShader;
    extern Shader prefilterShader;
    extern Shader brdfShader;
    extern Shader passthroughShader;
    extern Shader spriteShader;
    extern Shader textShader;
    extern Shader ShadowShader;
    extern Shader deferredShader;
    extern Shader deferredAmbientShader;
    extern Shader debugShader;

    extern Shader tonemapShader;
    extern Shader fxaaShader;
    extern Shader gaussianBlurShader;
    extern Shader ssaoShader;
    extern Shader ssaoBlurShader;
    extern Shader ssrShader;



    extern Texture brdf;


    void loadDefaultResources();

    Texture &getTexture(const Path &path, bool pixelated=false);

    Shader &getShader(const Path &vertexShader, const Path &fragmentShader);

    Shader &getShader(const Path &vertexShader, const Path &fragmentShader, const Path &geometryShader);

    AssimpFile &getAssimpFile(const Path &path);

    Material &getMaterial(const Path &path);
}