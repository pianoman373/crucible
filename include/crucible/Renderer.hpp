#pragma once

#include <crucible/Math.hpp>
#include <crucible/Material.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Camera.hpp>
#include <crucible/DebugRenderer.hpp>
#include <crucible/Frustum.hpp>
#include <crucible/IRenderable.hpp>
#include <crucible/PostProcessing.hpp>
#include <crucible/Bone.hpp>
#include <crucible/Mesh.hpp>
#include <crucible/Model.hpp>

#include <vector>

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};

namespace Renderer {

	extern DebugRenderer debug;

	extern Cubemap irradiance;
	extern Cubemap specular;

    extern std::vector<std::shared_ptr<PostProcessor>> postProcessingStack;

    /**
     * Sets up vital shaders and variables only once at startup.
     */
    void init(bool shadows, int shadowResolution, int resolutionX, int resolutionY);

    void resize(int resolutionX, int resolutionY);

    void matchWindowResolution(float scale=1.0f);

	/**
	 * Pushes a point light to the render buffer for the next flush event.
	 */
    void renderPointLight(const vec3 &position, const vec3 &color, float radius);

    /**
     * General purpose abstraction of all render calls to an internal renderer.
     */
    void render(const IRenderable *mesh, const Material *material, const Transform *transform, const AABB *aabb=nullptr, const Bone *bones=nullptr);

    /**
     * Same as the general purpose render command, but accepts Models.
     */
    void render(const Model *model, const Transform *transform, const AABB *aabb=nullptr);

    void renderSkybox(const Material *material);

    void renderGbuffers(const Camera &cam, const Frustum &f, bool doFrustumCulling);

    void renderForwardPass(const Camera &cam, const Frustum &f, bool doFrustumCulling);

    void lightGbuffers(const Camera &cam);

	Cubemap renderToProbe(const vec3 &position);

	/**
	* Flush command with frustum culling disabled.
	*/
	void flush(const Camera &cam);

	/**
     * In order to allow render to be called from anywhere at any time, render calls
     * are put in a buffer to be drawn later. Calling flush renders them all at once and
     * clears the buffer for the next time.
     */
    void flush(const Camera &cam, const Frustum &f, bool doFrustumCulling = true);

    const Texture &flushToTexture(const Camera &cam);

	const Texture &flushToTexture(const Camera &cam, const Frustum &f, bool doFrustumCulling = true);

    void renderToFramebuffer(const Camera &cam, const Frustum &f, bool doFrustumCulling = true);

    void setSun(const DirectionalLight &light);

    void setClearColor(vec3 color);

    vec2i getResolution();

    Framebuffer &getGBuffer();
};
