#include <crucible/crucible.hpp>

struct point {
    vec3 position;
    vec3 velocity;
};

static std::vector<point> points;
static Mesh debugRendererMesh;

void updateMesh() {
    debugRendererMesh.positions.clear();

    for (int i = 0; i < points.size(); i++) {
        debugRendererMesh.positions.push_back(points[i].position);
    }

    debugRendererMesh.generate();
};

int main(int argc, char *argv[]) {
    Window::create({1000, 800}, "test", false);
    Camera cam(vec3(1.0f, 1.0f, 6.0f));

    Shader s;
    s.loadFile("resources/points.vsh", "resources/points.fsh");


    debugRendererMesh.renderMode = GL_POINTS;

    for (int i = 0; i < 10000; i++) {

        float angle = rand() * 3.141592f * 2.0f;

        float x = (float)sin(angle) * Util::rand();
        float z = (float)cos(angle) * Util::rand();

        float y = 0;

        vec3 velocity = cross(normalize(vec3(x, y, z)), vec3(0.0f, 1.0f, 0.0f)) * 0.01f;

        points.push_back({vec3(x, y, z), velocity});
    }

    debugRendererMesh.generate();

    Renderer::init(false, 0, 1000, 800);

    Material mat;
    mat.setShader(s);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPointSize(2.0f);

    while (Window::isOpen()) {
        Window::begin();
        cam.dimensions = {(float)Window::getWindowSize().x, (float)Window::getWindowSize().y};
        Util::updateSpaceCamera(cam);

        for (int i = 0; i < points.size(); i++) {
            float distance = length(points[i].position);
            points[i].velocity = points[i].velocity + (0.00001f / (distance*distance));

            points[i].position = points[i].position + points[i].velocity;
        }

        updateMesh();

        Renderer::render(&debugRendererMesh, &mat, Transform(), AABB());
        Renderer::flush(cam);

        Window::end();
    }
}
