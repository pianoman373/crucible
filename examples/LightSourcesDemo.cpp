#include <crucible/crucible.hpp>

int main() {
	Window::create({ 1280, 720 }, "Light Sources Demo", false, false);
    Renderer::init(true, 2048, 1280, 720);

    
    Renderer::pushPostProcessor(new SsaoPostProcessor());
    Renderer::pushPostProcessor(new BloomPostProcessor());
    Renderer::pushPostProcessor(new TonemapPostProcessor());
    Renderer::pushPostProcessor(new FxaaPostProcessor());

	// Cubemap cubemap;
	// cubemap.loadEquirectangular("resources/canyon.hdr");
    // Renderer::environment = cubemap;
    // IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);
	Renderer::setSun({ vec3(1.05f, -1.2f, -1.3f), vec3(1.2f) });

	Mesh shaderball = Resources::getAssimpFile("resources/shaderball.fbx").getMesh(0);
    Mesh cube = Primitives::cube(0.1f, 20.0f, 1.0f, 20.0f);
    Mesh sphere = Primitives::sphere(32, 32);

    Material light;
    light.setPBRUniforms(vec3(1.0f, 0.6f, 0.4f), 0.0f, 0.0f);
    light.setUniformFloat("emission", 3.0f);

	Material &wood = Resources::getMaterial("resources/wood.crmaterial");
    Material &plastic = Resources::getMaterial("resources/plastic.crmaterial");
    Material &rustediron = Resources::getMaterial("resources/rustediron.crmaterial");
    Material &gold = Resources::getMaterial("resources/gold.crmaterial");
    Material &ceramic = Resources::getMaterial("resources/ceramic.crmaterial");
    Material &checker = Resources::getMaterial("resources/checkerboard.crmaterial");

    Transform shaderballTransform(vec3(0.0f, 1.9f, 0.0f), quaternion(), vec3(0.25f));
    Transform groundTransform(vec3(0.0f, -0.5f, 0.0f), quaternion(), vec3(1.0f));
    Transform lightTransform(vec3(0.0f, 2.0f, 5.0f), quaternion(), vec3(0.25f));

    Camera cam;

    while (Window::isOpen()) {
        Window::begin();
        cam.matchWindowResolution();
        Renderer::matchWindowResolution();

        cam.position = vec3(sin(Window::getTime()*0.1f), 0.3f, cos(Window::getTime()*0.1f)) * 8.0f;
        cam.direction = normalize(-cam.position);

        Renderer::render(shaderball, rustediron, shaderballTransform);
        Renderer::render(cube, checker, groundTransform);



        Renderer::renderPointLight(vec3(0.0f, 2.0f, 5.0f), vec3(1.0f, 0.6f, 0.4f)*10.0f, 5.0f);
        Renderer::debug.renderDebugSphere(lightTransform.position, 5.0f, vec3(1.0f, 0.6f, 0.4f));

        Renderer::render(sphere, light, lightTransform);

        Renderer::flush(cam);
        Window::end();
    }
}