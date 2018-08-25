#include <crucible/PostProcessor.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/Window.hpp>
#include <crucible/InternalShaders.hpp>

#include <glad/glad.h>

#include <random>

void PostProcessor::doBloom(const Texture &deferred) {
    glViewport(0, 0, bloomBuffer0.getWidth(), bloomBuffer0.getHeight());
    bloomBuffer1.bind();
    Renderer::passthroughShader.bind();
    deferred.bind();
    Renderer::framebufferMesh.render();

    bloomBuffer0.bind();
    gaussianBlurShader.bind();
    gaussianBlurShader.uniformBool("horizontal", true);
    bloomBuffer1.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    bloomBuffer1.bind();
    gaussianBlurShader.bind();
    gaussianBlurShader.uniformBool("horizontal", false);
    bloomBuffer0.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    // -------------------------------------------------------------------------

    glViewport(0, 0, bloomBuffer3.getWidth(), bloomBuffer3.getHeight());
    bloomBuffer3.bind();
    Renderer::passthroughShader.bind();
    bloomBuffer1.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    bloomBuffer2.bind();
    gaussianBlurShader.bind();
    gaussianBlurShader.uniformBool("horizontal", true);
    bloomBuffer3.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    bloomBuffer3.bind();
    gaussianBlurShader.bind();
    gaussianBlurShader.uniformBool("horizontal", false);
    bloomBuffer2.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    // -------------------------------------------------------------------------

    glViewport(0, 0, bloomBuffer5.getWidth(), bloomBuffer5.getHeight());
    bloomBuffer5.bind();
    Renderer::passthroughShader.bind();
    bloomBuffer3.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    bloomBuffer4.bind();
    gaussianBlurShader.bind();
    gaussianBlurShader.uniformBool("horizontal", true);
    bloomBuffer5.getAttachment(0).bind();
    Renderer::framebufferMesh.render();

    bloomBuffer5.bind();
    gaussianBlurShader.bind();
    gaussianBlurShader.uniformBool("horizontal", false);
    bloomBuffer4.getAttachment(0).bind();
    Renderer::framebufferMesh.render();
}



void PostProcessor::init() {
    tonemapShader.loadPostProcessing(InternalShaders::tonemap_glsl);
    fxaaShader.loadPostProcessing(InternalShaders::fxaa_glsl);
    gaussianBlurShader.loadPostProcessing(InternalShaders::gaussianBlur_glsl);
    ssaoShader.loadPostProcessing(InternalShaders::ssao_glsl);
    ssaoBlurShader.loadPostProcessing(InternalShaders::ssaoBlur_glsl);
    ssrShader.loadPostProcessing(InternalShaders::SSR_glsl);

    //setup SSAO
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 256; ++i)
    {
        vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)
        );
        sample  = normalize(sample);
        sample = sample * randomFloats(generator);
        float scale = (float)i / 256.0;

        scale   = lerp(0.1f, 1.0f, scale * scale);
        sample = sample * scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    noiseTex.setID(noiseTexture);

    resize();
}

void PostProcessor::resize() {
    vec2i resolution = Renderer::getResolution();

    // HDR framebuffer 0
            HDRbuffer0.destroy();
    HDRbuffer0.setup(resolution.x, resolution.y);
    HDRbuffer0.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
    // HDR framebuffer 1
    HDRbuffer1.destroy();
    HDRbuffer1.setup(resolution.x, resolution.y);
    HDRbuffer1.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    //bloom framebuffers
    bloomBuffer0.destroy();
    bloomBuffer0.setup(resolution.x/2, resolution.y/2);
    bloomBuffer0.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);
    bloomBuffer1.destroy();
    bloomBuffer1.setup(resolution.x/2, resolution.y/2);
    bloomBuffer1.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);

    bloomBuffer2.destroy();
    bloomBuffer2.setup(resolution.x/8, resolution.y/8);
    bloomBuffer2.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);
    bloomBuffer3.destroy();
    bloomBuffer3.setup(resolution.x/8, resolution.y/8);
    bloomBuffer3.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);

    bloomBuffer4.destroy();
    bloomBuffer4.setup(resolution.x/32, resolution.y/32);
    bloomBuffer4.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);
    bloomBuffer5.destroy();
    bloomBuffer5.setup(resolution.x/32, resolution.y/32);
    bloomBuffer5.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);

    ssrBlurBuffer0.destroy();
    ssrBlurBuffer0.setup(resolution.x/8, resolution.y/8);
    ssrBlurBuffer0.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);

    ssrBlurBuffer1.destroy();
    ssrBlurBuffer1.setup(resolution.x/8, resolution.y/8);
    ssrBlurBuffer1.attachTexture(GL_RGBA16F, GL_RGB, GL_FLOAT);

    ssaoBuffer.destroy();
    ssaoBuffer.setup(resolution.x, resolution.y);
    ssaoBuffer.attachTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);

    ssaoBufferBlur.destroy();
    ssaoBufferBlur.setup(resolution.x, resolution.y);
    ssaoBufferBlur.attachTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
}

Texture PostProcessor::postRender(const Camera &cam, const Texture &deferred, const Texture &gPosition, const Texture &gNormal, const Texture &gAlbedo, const Texture &gRoughnessMetallic) {
    vec2i resolution = Renderer::getResolution();

    if (Renderer::settings.ssao) {
        // render the g-buffers for SSAO
        // ---------------------------------------------
        ssaoBuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ssaoShader.bind();

        ssaoShader.uniformInt("gPosition", 0);
        gPosition.bind(0);

        ssaoShader.uniformInt("gNormal", 1);
        gNormal.bind(1);

        ssaoShader.uniformInt("texNoise", 2);
        noiseTex.bind(2);

        ssaoShader.uniformMat4("projection", cam.getProjection());
        ssaoShader.uniformFloat("radius", Renderer::settings.ssaoRadius);
        ssaoShader.uniformInt("kernelSize", Renderer::settings.ssaoKernelSize);
        ssaoShader.uniformVec3("noiseScale",  vec3(resolution.x/4.0f, resolution.y/4.0f, 0.0f));

        for (int i = 0; i < Renderer::settings.ssaoKernelSize; i++) {
            ssaoShader.uniformVec3(std::string("samples[") + std::to_string(i) + std::string("]"), ssaoKernel[i]);
        }
        Renderer::framebufferMesh.render();

        ssaoBufferBlur.bind();
        ssaoBlurShader.bind();
        ssaoBuffer.getAttachment(0).bind();
        Renderer::framebufferMesh.render();
    }

    // SSR
    if (Renderer::settings.SSR){
        glViewport(0, 0, ssrBlurBuffer0.getWidth(), ssrBlurBuffer0.getHeight());
        ssrBlurBuffer0.bind();
        Renderer::passthroughShader.bind();
        deferred.bind();
        Renderer::framebufferMesh.render();

        ssrBlurBuffer1.bind();
        gaussianBlurShader.bind();
        gaussianBlurShader.uniformBool("horizontal", true);
        ssrBlurBuffer0.getAttachment(0).bind();
        Renderer::framebufferMesh.render();

        ssrBlurBuffer0.bind();
        gaussianBlurShader.bind();
        gaussianBlurShader.uniformBool("horizontal", false);
        ssrBlurBuffer1.getAttachment(0).bind();
        Renderer::framebufferMesh.render();

        glViewport(0, 0, resolution.x, resolution.y);

        HDRbuffer0.bind();
        ssrShader.bind();

        ssrShader.uniformMat4("view", cam.getView());
        ssrShader.uniformMat4("projection", cam.getProjection());


        ssrShader.uniformInt("gPosition", 0);
        gPosition.bind(0);

        ssrShader.uniformInt("gNormal", 1);
        gNormal.bind(1);

        ssrShader.uniformInt("gAlbedo", 2);
        gAlbedo.bind(2);

        ssrShader.uniformInt("gRoughnessMetallic", 3);
        gRoughnessMetallic.bind(3);

        ssrShader.uniformInt("deferred", 4);
        deferred.bind(4);

        ssrShader.uniformInt("deferredBlur", 5);
        ssrBlurBuffer0.getAttachment(0).bind(5);

        ssrShader.uniformInt("prefilter", 6);
        Renderer::specular.bind(6);
        ssrShader.uniformInt("brdf", 7);
        Renderer::brdf.bind(7);

        Renderer::framebufferMesh.render();
    }

    // bloom
    // ---------------

    if (Renderer::settings.bloom) {
        doBloom(deferred);

        glViewport(0, 0, resolution.x, resolution.y);
    }

    // post processing
    // ---------------
    HDRbuffer1.bind();

    bloomBuffer1.getAttachment(0).bind(0);
    bloomBuffer3.getAttachment(0).bind(1);
    bloomBuffer5.getAttachment(0).bind(2);

    if (Renderer::settings.SSR) {
        HDRbuffer0.getAttachment(0).bind(3);
    }
    else {
        deferred.bind(3);
    }

    ssaoBufferBlur.getAttachment(0).bind(4);

    tonemapShader.bind();
    tonemapShader.uniformBool("vignette", Renderer::settings.vignette);
    tonemapShader.uniformBool("tonemap", Renderer::settings.tonemap);
    tonemapShader.uniformBool("bloom", Renderer::settings.bloom);
    tonemapShader.uniformBool("ssao", Renderer::settings.ssao);
    tonemapShader.uniformFloat("bloomStrength", Renderer::settings.bloomStrength);
    tonemapShader.uniformInt("bloom0", 0);
    tonemapShader.uniformInt("bloom1", 1);
    tonemapShader.uniformInt("bloom2", 2);
    tonemapShader.uniformInt("deferred", 3);
    tonemapShader.uniformInt("ssaoTexture", 4);

    Renderer::framebufferMesh.render();


    // fxaa
    // -------------------------------
    if (Renderer::settings.fxaa) {
        HDRbuffer0.bind();
        glViewport(0, 0, resolution.x, resolution.y);
        fxaaShader.bind();
        HDRbuffer1.getAttachment(0).bind();
        Renderer::framebufferMesh.render();

        return HDRbuffer0.getAttachment(0);
    }

    return HDRbuffer1.getAttachment(0);
}