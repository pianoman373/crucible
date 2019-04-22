#include <crucible/crucible.hpp>

int main() {
    // set up renderer
	Window::create(vec2i(1280, 720), "Hello Window Demo", false, false);
    Renderer::init(1280, 720);
    Renderer::setClearColor(vec3(0.01f));

    Camera cam;


    // create all resources needed to render and light a basic sphere
    Material material;
    material.setPBRUniforms(vec3(0.5f), 0.5f, 0.0f);

    Mesh sphere = Primitives::sphere(32, 32);

    Transform transform(vec3(0.0f, 0.0f, -3.0f));

    DirectionalLight sun(vec3(1.05f, -1.2f, -1.3f), vec3(1.2f));

    while (Window::isOpen()) {
        Window::begin();

        // dynamically scale reesolution to window size
        cam.matchWindowResolution();
        Renderer::matchWindowResolution();

        // render objects and lights
        Renderer::render(&sphere, &material, &transform);
        Renderer::renderDirectionalLight(&sun);

        Renderer::flush(cam);
        Window::end();
    }
}