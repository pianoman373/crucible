#include <crucible/crucible.hpp>

int main() {
    // set up renderer
	Window::create(vec2i(1280, 720), "N-Body Demo", false, false);
    Renderer::init(1280, 720);
    Renderer::setClearColor(vec3(0.0f));

    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new BloomPostProcessor())); // Bloom
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new TonemapPostProcessor())); // Tonemapping
    Renderer::postProcessingStack.push_back(std::shared_ptr<PostProcessor>(new FxaaPostProcessor())); // FXAA

    Camera cam;
    cam.position = vec3(0.0f, 2.0f, 5.0f);

    ParticleSystem particles;
    particles.sorting = false;
    particles.particleCount = 100000;

    particles.spawnCallback = [&] () {
        ParticleInfo p;

        p.position = vec3((randf()*10.0f) + 3.0f, 0.0f, 0.0f);
        p.velocity = vec3(srandf()*0.1f, srandf()*0.1f, randf()*2.0f);

        p.size = 0.01f;

        return p;
    };

    particles.updateCallback = [&] (ParticleInfo &p, float delta) {
        
        p.position = p.position + (p.velocity * delta);

        float distance = length(p.position);
        p.velocity = p.velocity - (normalize(p.position) * delta * 100.0f * (1.0f / (distance*distance)));

        p.color = vec3(0.0f, 0.2f, 1.5f);
        p.color.x += length(p.velocity) * 0.5f;

        p.size = 0.05f;
    };

    particles.init();
    particles.setTexture(Resources::getTexture("resources/point.png"));

    while (Window::isOpen()) {
        Window::begin();

        cam.updateSpaceCamera(5.0f);

        // dynamically scale reesolution to window size
        cam.matchWindowResolution();
        Renderer::matchWindowResolution();

        particles.update(cam);
        particles.render();

        Renderer::flush(cam);
        Window::end();
    }
}