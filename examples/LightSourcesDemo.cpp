#include <crucible/crucible.hpp>

int main() {
    // set up renderer
	Window::create(vec2i(1280, 720), "Light Sources Demo", false, false);
    Renderer::init(1280, 720);
    Renderer::setClearColor(vec3(0.01f));
    
    // add post processing effects
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new SsaoPostProcessor())); // SSAO
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new BloomPostProcessor())); // Bloom
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new TonemapPostProcessor())); // Tonemapping
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new FxaaPostProcessor())); // FXAA

    // store scene lighting
    DirectionalLight sun(vec3(1.05f, -1.2f, -1.3f), vec3(1.2f));
    PointLight lamp(vec3(0.0f, 2.0f, 5.0f), vec3(1.0f, 0.6f, 0.4f)*10.0f, 5.0f);

    // create all meshes to be used in the scene
	Mesh shaderball = Resources::getAssimpFile("resources/shaderball.fbx").getMesh(0);
    Mesh cube = Primitives::cube(0.1f, 20.0f, 1.0f, 20.0f);
    Mesh sphere = Primitives::sphere(32, 32);

    // create all materials to be used in the scene
    Material light;
    light.setPBRUniforms(vec3(1.0f, 0.6f, 0.4f), 0.0f, 0.0f);
    light.setUniformFloat("emission", 3.0f);

    Material &rustediron = Resources::getMaterial("resources/rustediron.crmaterial");
    Material &checker = Resources::getMaterial("resources/checkerboard.crmaterial");

    // store all the transforms in the scene
    Transform shaderballTransform(vec3(0.0f, 1.9f, 0.0f), quaternion(), vec3(0.25f));
    Transform groundTransform(vec3(0.0f, -0.5f, 0.0f), quaternion(), vec3(1.0f));
    Transform lightTransform(vec3(0.0f, 2.0f, 5.0f), quaternion(), vec3(0.25f));

    Camera cam;

    while (Window::isOpen()) {
        Window::begin();

        // dynamically scale reesolution to window size
        cam.matchWindowResolution();
        Renderer::matchWindowResolution();

        // orbit camera around center
        cam.position = vec3(sin(Window::getTime()*0.1f), 0.3f, cos(Window::getTime()*0.1f)) * 8.0f;
        cam.direction = normalize(-cam.position);

        // render floor and shaderball
        Renderer::render(&shaderball, &rustediron, &shaderballTransform);
        Renderer::render(&cube, &checker, &groundTransform);

        // render point light along with a sphere representing it
        Renderer::renderPointLight(&lamp);
        Renderer::render(&sphere, &light, &lightTransform);

        // apply sun light to the scene
        Renderer::renderDirectionalLight(&sun);

        Renderer::flush(cam);
        Window::end();
    }
}