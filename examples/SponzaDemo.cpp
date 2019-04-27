#include <crucible/crucible.hpp>

#include <glad/glad.h>

int main() {
    // set up renderer
	Window::create({ 1280, 720 }, "Sponza Demo", false, false);
    Renderer::init(1280, 720);

    // add post processing effects
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new SsaoPostProcessor())); // SSAO
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new BloomPostProcessor())); // Bloom
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new TonemapPostProcessor())); // Tonemapping
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new FxaaPostProcessor())); // FXAA

	Camera cam;
    cam.position = vec3(-30.0f, 10.0f, 0.0f);
    cam.direction = vec3(1.0f, 0.0f, 0.0f);

    // store sun object
    DirectionalLight sun(normalize(vec3(1.4f, -5.8f, -1.0f)), vec3(1.0f, 1.0f, 1.0f) * 5.0f, 2048, 3, 150.0f);

    // load skybox
	Cubemap cubemap;
	cubemap.loadEquirectangular("resources/canyon.hdr");

    Mesh sphere = Primitives::sphere(16, 16);
    Material probe;
    probe.setPBRUniforms(vec3(1.0f), 0.0f, 1.0f);

    Mesh dragon = Resources::getAssimpFile("resources/dragon.obj").getMesh(0);
    Material metal;
    metal.setPBRUniforms(vec3(1.0f), 0.3f, 1.0f);

    // skybox material
    Material skybox;
    skybox.deferred = false;
    skybox.setShader(Resources::cubemapShader);
    skybox.setUniformCubemap("environmentMap", cubemap);

    // add all objects to the scene
    Scene scene;
    scene.setupPhysicsWorld();

    Resources::getAssimpFile("resources/sponza/sponza.obj").addToScene(scene);
    
    scene.createMeshObject(sphere, probe, Transform(vec3(0.0f, 20.0f, 0.0f)), "probe");
    scene.createMeshObject(dragon, metal, Transform(vec3(20.0f, 4.5f, -2.0f), quaternion(vec3(0.0f, 1.0f, 0.0f), radians(30)), vec3(1.0f)), "dragon");

    bool first = true;

    while (Window::isOpen()) {
        Window::begin();

        // dynamically scale reesolution to window size
        cam.matchWindowResolution();
        Renderer::matchWindowResolution();

        cam.updateFpsCamera(20.0f);

        //render skybox and sunlight
        Renderer::renderSkybox(&skybox);
        

        // update scene physics
        scene.update(Window::deltaTime());

        // render all objects in the scene
        scene.render();

        // if this is the first frame, render scene into a light probe
        if (first) {
            IBL::generateIBLmaps(vec3(0.0f,  20000.0f, 0.0f), Renderer::irradiance, Renderer::specular);

            IBL::generateIBLmaps(vec3(0.0f,  20.0f, 0.0f), Renderer::irradiance, Renderer::specular);
            
            first = false;
        }

        Renderer::renderDirectionalLight(&sun);
        
        // render the scene to the screen
        Renderer::flush(cam);

        Window::end();
    }
}