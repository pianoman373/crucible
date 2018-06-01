#include <crucible/crucible.hpp>
#include <crucible/Path.hpp>

#include "WorkspaceObject.hpp"
#include "GuiMain.hpp"

void orbitCamera(Camera &cam) {
    static vec3 camDirection = vec3(0.0f, 0.0f, 1.0f);
    static float camDistance = 10;
    static vec3 camOffset = vec3();

    cam.position = (camDirection * camDistance) + camOffset;
    cam.direction = normalize(camOffset - cam.position);



    if (Input::isKeyDown(Input::KEY_LEFT_SHIFT)) {
        camOffset.y += Input::getScroll() * 0.1f * camDistance;
    }
    else {
        camDistance -= Input::getScroll() * 0.1f * camDistance;
    }

    Window::setMouseGrabbed(Input::isMouseButtonDown(1));

    static vec2 lastMousePos;

    if (Input::isMouseButtonDown(1)) {

        vec2 offset = Input::getCursorPos() - lastMousePos;
        float xOffset = -offset.x / 10.0f;
        float yOffset = offset.y / 10.0f;


        //vertical
        mat4 vmat;
        vmat = rotate(vmat, cam.getRight(), -yOffset);

        //horizontal
        mat4 hmat = mat4();
        hmat = rotate(hmat, vec3(0.0f, 1.0f, 0.0f), xOffset);

        camDirection = normalize(vec3(hmat * vmat * vec4(camDirection, 1.0f)));
    }
    
    lastMousePos = Input::getCursorPos();
}

static WorkspaceObject workspace;

static Cubemap cubemap;
static Texture cubemapRaw;


int main() {
    Window::create({ 1280, 720 }, "test", false);
    Camera cam(vec3(0.0f, 0.0f, 0.0f));


    Renderer::init(true, 2048, 1280, 720);
    Renderer::settings.tonemap = true;
    Renderer::settings.vignette = false;
    Renderer::settings.bloom = true;
    Renderer::settings.ssao = true;


    cubemap.loadEquirectangular("resources/canyon.hdr");
    cubemapRaw.load("resources/canyon.hdr");

    Renderer::environment = cubemap;
    Renderer::generateIBLmaps();
    cubemap.setID(0);
    //Renderer::environment = cubemap;

    Renderer::setSun({ vec3(1.05f, -6.8f, -1.3f), vec3(10.0f, 9.5f, 9.0f) });

    workspace.open(Path::getFullPath("resources/Rifle/rifle.asset"));

    GuiMain gui(&workspace);

    while (Window::isOpen()) {
        Window::begin();
        cam.dimensions = {1920, 1080};
        orbitCamera(cam);

        workspace.render(gui.selected);

        Renderer::flush(cam);
        gui.render();

        Window::end();
    }
}
