#pragma once

#include <crucible/Math.hpp>
#include <crucible/Material.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Camera.hpp>
#include <crucible/DebugRenderer.hpp>

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
};

struct RendererSettings {
	bool fxaa = true;
	bool vignette = true;
	bool tonemap = true;
	bool bloom = true;
	float bloomStrength = 0.05f;
	bool ssao = true;
	float ssaoRadius = 10.0f;
	int ssaoKernelSize = 64;
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
	extern unsigned int brdf;

	extern std::vector<PointLight> pointLights;
    extern std::vector<DirectionalLight> directionalLights;

	Texture getTexture(std::string path);

    /**
     * Sets up vital shaders and variables only once at startup.
     */
    void init(bool shadows, int shadowResolution, int resolutionX, int resolutionY);

	void renderSkybox(mat4 view, mat4 projection);

    /**
     * General purpose abstraction of all render calls to an internal renderer.
     */
    void render(Mesh *mesh, Material *material, Transform transform, AABB aabb);

    void render(Model *model, Transform transform, AABB aabb);

    void renderSprite(Texture tex, vec2 pos, vec2 dimensions, vec4 uv);

    /**
     * Make all future render calls render with an outline.
     */
     void enableOutline();

     /**
      * Disable outlineing called with enableOutline.
      */
     void disableOutline();

    /**
     * In order to allow render to be called from anywhere at any time, render calls
     * are put in a buffer to be drawn later. Calling flush renders them all at once and
     * clears the buffer for the next time.
     */
    void flush(Camera cam);

    void setSkyboxShader(Shader s);

    void setSun(DirectionalLight light);

	void generateIBLmaps();
};
