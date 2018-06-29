#include <crucible/crucible.hpp>
#include "SimplexNoise.hpp"

static float noise(vec2 position, int octaves, float frequency, float persistence) {
    float total = 0.0;
    float maxAmplitude = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        total += SimplexNoise::noise(position.x * frequency, position.y * frequency) * amplitude;
        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }
    return total / maxAmplitude;
}

static float ridgedNoise(vec2 position, int octaves, float frequency, float persistence) {
    float total = 0.0f;
    float maxAmplitude = 0.0f;
    float amplitude = 1.0f;
    for (int i = 0; i < octaves; i++) {
        total += ((1.0f - fabs(SimplexNoise::noise(position.x * frequency, position.y * frequency))) * 2.0f - 1.0f) * amplitude;
        frequency *= 2.0f;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

static float heightAt(int x, int z) {
    return ridgedNoise({x / 100.0f, z / 100.0f}, 8, 0.1f, 0.5f) * 100.0f;
}

static void generateTerrainMesh(Mesh &m) {
    int i = 0;

    const int size = 1000;

    for (int x = 0; x < size; x++) {
        for (int z = 0; z < size; z++) {
            float height =  heightAt(x, z);
            m.positions.push_back({x, height, z});

            vec3 lineNegX = normalize(vec3(x - 1, heightAt(x-1, z), z) - vec3(x, height, z));
            vec3 linePosX = normalize(vec3(x + 1, heightAt(x+1, z), z) - vec3(x, height, z));

            vec3 lineNegY = normalize(vec3(x, heightAt(x, z-1), z - 1) - vec3(x, height, z));
            vec3 linePosY = normalize(vec3(x, heightAt(x, z+1), z + 1) - vec3(x, height, z));


            vec3 norm1 = cross(lineNegY, lineNegX);
            vec3 norm2 = cross(linePosX, lineNegY);
            vec3 norm3 = cross(linePosY, linePosX);
            vec3 norm4 = cross(lineNegX, linePosY);

            vec3 finalNorm = (norm1 + norm2 + norm3 + norm4) / vec3(4, 4, 4);

            m.normals.push_back(finalNorm);

            if (x < size-1 && z < size-1) {
                m.indices.push_back(i);
                m.indices.push_back(i+1);
                m.indices.push_back(i+size);

                m.indices.push_back(i+1);
                m.indices.push_back(i+size+1);
                m.indices.push_back(i+size);
            }
            i++;
        }
    }

    m.generate();
}

int main() {
    Window::create({ 1280, 720 }, "test", false);
    Camera cam(vec3(0.0f, 150.0f, 0.0f));

    Renderer::init(true, 2048, 1280, 720);

    //Renderer::settings.bloom = false;

    Shader skyboxShader;
    skyboxShader.loadFile("resources/skybox.vsh", "resources/skybox.fsh");

    Renderer::setSkyboxShader(skyboxShader);

    Renderer::setSun({ vec3(1.05f, -0.5f, -1.3f), vec3(10.0f, 8.0f, 7.0f) });

    IBL::generateIBLmaps(Renderer::irradiance, Renderer::specular);

    Material ground;
    ground.setShader(Renderer::standardShader);
    ground.setPBRUniforms(vec3(0.1f), 0.5f, 0.0f);

    Material ball;
    ball.setShader(Renderer::standardShader);
    ball.setPBRUniforms(vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);

    Mesh cube;
    Primitives::cube(cube);

    Mesh sphere;
    Primitives::sphere(sphere, 32, 32);

    Mesh terrain;
    generateTerrainMesh(terrain);

    while (Window::isOpen()) {
        Window::begin();
        cam.dimensions = {(float)Window::getWindowSize().x, (float)Window::getWindowSize().y};
        Util::updateSpaceCamera(cam, 20.0f);

        Renderer::render(&sphere, &ball, Transform(vec3(0.0f, 1.5f, 0.0f), quaternion(), vec3(1.0f)), AABB());


        Renderer::render(&terrain, &ground, Transform(vec3(), quaternion(), vec3(1.0f)), AABB());

        Renderer::flush(cam);

        Window::end();
    }
}