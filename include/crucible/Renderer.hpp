#pragma once

#include <crucible/Math.hpp>
#include <crucible/Material.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Camera.hpp>
#include <crucible/DebugRenderer.hpp>
#include <crucible/Frustum.hpp>
#include <crucible/IRenderable.hpp>
#include <crucible/PostProcessor.hpp>
#include <crucible/Bone.hpp>

#include <vector>

class Mesh;
class AABB;
class Material;
class Model;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};

struct RendererSettings {
	bool fxaa = true;
	bool vignette = true;
	bool tonemap = true;
	bool bloom = true;
	bool SSR = false;
	float bloomStrength = 0.05f;
	bool ssao = true;
	float ssaoRadius = 10.0f;
	int ssaoKernelSize = 8;
};

class Renderer {
public:
	static RendererSettings settings;

	static vec3 ambient;

	static DebugRenderer debug;

	static Mesh cubemapMesh;
	static Mesh framebufferMesh;

	static Shader standardShader;
	static Shader eq2cubeShader;
	static Shader cubemapShader;
	static Shader irradianceShader;
	static Shader prefilterShader;
	static Shader brdfShader;
	static Shader outlineShader;
	static Shader passthroughShader;

	static Cubemap environment;
	static Cubemap irradiance;
	static Cubemap specular;

	static Texture brdf;

	static PostProcessor postProcessor;

private:
    static mat4 shadowMatrix(float radius, const Camera &cam, float depth);
    static Frustum shadowFrustum(float radius, const Camera &cam, float depth);
    static void renderShadow(Framebuffer &fbuffer, mat4 lightSpaceMatrix, Frustum f, bool doFrustumCulling);
    static void renderDebugGui();

public:
	static void resize(int resolutionX, int resolutionY);

    /**
     * Sets up vital shaders and variables only once at startup.
     */
    static void init(bool shadows, int shadowResolution, int resolutionX, int resolutionY);

    /**
     * Mostly internal function that will only render the skybox and nothing else. This doesn't need to be called under
     * normal rendering circumstances.
     */
    static void renderSkybox(const mat4 &view, const mat4 &projection, const vec3 &cameraPos = {0, 0, 0});

	/**
	 * Pushes a point light to the render buffer for the next flush event.
	 */
    static void renderPointLight(const vec3 &position, const vec3 &color, float radius);

    /**
     * General purpose abstraction of all render calls to an internal renderer.
     */
    static void render(const IRenderable *mesh, const Material *material, const Transform *transform, const AABB *aabb=nullptr, const Bone *bones=nullptr);

    /**
     * Same as the general purpose render command, but accepts Models.
     */
    static void render(const Model *model, const Transform *transform, const AABB *aabb=nullptr);


    // macro functions for usability
    inline static void render(const IRenderable &mesh, const Material &material, const Transform &transform) {
        render(&mesh, &material, &transform, nullptr, nullptr);
    }

    inline static void render(const IRenderable &mesh, const Material &material, const Transform &transform, const AABB &aabb) {
        render(&mesh, &material, &transform, &aabb, nullptr);
    }

    inline static void render(const IRenderable &mesh, const Material &material, const Transform &transform, const Bone &bones) {
        render(&mesh, &material, &transform, nullptr, &bones);
    }

    inline static void render(const IRenderable &mesh, const Material &material, const Transform &transform, const AABB &aabb, const Bone &bones) {
        render(&mesh, &material, &transform, &aabb, &bones);
    }



    inline static void render(const Model &model, const Transform &transform) {
        render(&model, &transform, nullptr);
    }

    inline static void render(const Model &model, const Transform &transform, const AABB &aabb) {
        render(&model, &transform, &aabb);
    }

    static void renderGbuffers(const Camera &cam, const Frustum &f, bool doFrustumCulling, Texture &gPosition,
                               Texture &gNormal, Texture &gAlbedo, Texture &gRoughnessMetallic);

    static Texture lightGbuffers(const Camera &cam, const Texture &gPosition, const Texture &gNormal,
                                 const Texture &gAlbedo, const Texture &gRoughnessMetallic);

     /**
      * Flush command with frustum culling disabled.
      */
      static void flush(const Camera &cam);

      static Cubemap renderToProbe(const vec3 &position);


	/**
     * In order to allow render to be called from anywhere at any time, render calls
     * are put in a buffer to be drawn later. Calling flush renders them all at once and
     * clears the buffer for the next time.
     */
    static void flush(const Camera &cam, const Frustum &f, bool doFrustumCulling = true);

    static void setSkyboxShader(const Shader &s);

    static void setSun(const DirectionalLight &light);

    static vec2i getResolution();
};
