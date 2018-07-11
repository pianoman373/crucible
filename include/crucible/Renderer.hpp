#pragma once

#include <crucible/Math.hpp>
#include <crucible/Material.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Camera.hpp>
#include <crucible/DebugRenderer.hpp>
#include <crucible/Frustum.hpp>
#include <crucible/IRenderable.hpp>
#include <crucible/PostProcessor.hpp>

#include <vector>

class Mesh;
class AABB;
class Material;
class Model;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
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

namespace Renderer {
	extern RendererSettings settings;

	extern vec3 ambient;

	extern DebugRenderer debug;

	extern Mesh cubemapMesh;
	extern Mesh framebufferMesh;

	extern Shader standardShader;
	extern Shader eq2cubeShader;
	extern Shader cubemapShader;
	extern Shader irradianceShader;
	extern Shader prefilterShader;
	extern Shader brdfShader;
	extern Shader outlineShader;
	extern Shader passthroughShader;

	extern Cubemap environment;
	extern Cubemap irradiance;
	extern Cubemap specular;

	extern Texture brdf;

	extern PostProcessor postProcessor;

	void resize(int resolutionX, int resolutionY);

    /**
     * Sets up vital shaders and variables only once at startup.
     */
    void init(bool shadows, int shadowResolution, int resolutionX, int resolutionY);

    /**
     * Mostly internal function that will only render the skybox and nothing else. This doesn't need to be called under
     * normal rendering circumstances.
     */
	void renderSkybox(mat4 view, mat4 projection, vec3 cameraPos={0, 0, 0});

	/**
	 * Pushes a point light to the render buffer for the next flush event.
	 */
	void renderPointLight(vec3 position, vec3 color, float radius);

    /**
     * General purpose abstraction of all render calls to an internal renderer.
     */
    void render(IRenderable *mesh, Material *material, Transform transform, AABB aabb=AABB());

    /**
     * Same as the general purpose render command, but accepts Models.
     */
    void render(Model *model, Transform transform, AABB aabb=AABB());


    /**
     * Make all future render calls render with an outline.
     */
     void enableOutline();

     /**
      * Disable outlineing called with enableOutline.
      */
     void disableOutline();

     void renderGbuffers(Camera cam, Frustum f, bool doFrustumCulling, Texture &deferred, Texture &gPosition, Texture &gNormal, Texture &gAlbedo, Texture &gRoughnessMetallic);

     /**
      * Flush command with frustum culling disabled.
      */
	void flush(Camera cam);


	/**
     * In order to allow render to be called from anywhere at any time, render calls
     * are put in a buffer to be drawn later. Calling flush renders them all at once and
     * clears the buffer for the next time.
     */
	void flush(Camera cam, Frustum f, bool doFrustumCulling=true);

    void setSkyboxShader(Shader s);

    void setSun(DirectionalLight light);

    vec2i getResolution();
}
