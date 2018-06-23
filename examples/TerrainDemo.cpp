#include <crucible/crucible.hpp>

static void generateTerrainMesh(Mesh &m) {
    int i = 0;

    for (int x = 0; x < 10; x++) {
        for (int z = 0; z < 10; z++) {
            m.positions.push_back({x, 0, z});

            m.normals.push_back({0.0f, 1.0f, 0.0f});

            if (x < 9 && z < 9) {
                m.indices.push_back(i);
                m.indices.push_back(i+1);
                m.indices.push_back(i+10);

                m.indices.push_back(i+1);
                m.indices.push_back(i+11);
                m.indices.push_back(i+10);
            }
            i++;
        }
    }

    m.generate();
}

int main() {
    Window::create({ 1280, 720 }, "test", false);
    Camera cam(vec3(0.0f, 15.0f, 0.0f));

    Renderer::init(true, 2048, 1280, 720);

    //Renderer::settings.bloom = false;

    Shader skyboxShader;
    skyboxShader.loadFile("resources/skybox.vsh", "resources/skybox.fsh");

    Renderer::setSkyboxShader(skyboxShader);

    Renderer::setSun({ vec3(1.05f, -0.1f, -1.3f), vec3(10.0f, 8.0f, 7.0f) });

    IBL::generateIBLmaps(Renderer::irradiance, Renderer::specular);

    Material ground;
    ground.setShader(Renderer::standardShader);
    ground.setPBRUniforms(vec3(0.1f), 0.5f, 0.0f);

    Material ball;
    ball.setShader(Renderer::standardShader);
    ball.setPBRUniforms(vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);

    Material glowy;
    glowy.setShader(Renderer::standardShader);
    glowy.setPBRUniforms(vec3(2.0f, 1.5f, 1.0f) * 0.5f, 0.0f, 0.0f);
    glowy.setUniformFloat("emission", 1.0f);

    Mesh cube;
    Primitives::cube(cube);

    Mesh sphere;
    Primitives::sphere(sphere, 32, 32);

    Mesh terrain;
    generateTerrainMesh(terrain);

    while (Window::isOpen()) {
        Window::begin();
        cam.dimensions = {(float)Window::getWindowSize().x, (float)Window::getWindowSize().y};
        Util::updateSpaceCamera(cam);

        Renderer::render(&sphere, &ball, Transform(vec3(0.0f, 1.5f, 0.0f), quaternion(), vec3(1.0f)), AABB());


        Renderer::render(&terrain, &ground, Transform(vec3(), quaternion(), vec3(1.0f)), AABB());

        Renderer::flush(cam);

        Window::end();
    }
}