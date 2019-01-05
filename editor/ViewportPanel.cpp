#include "ViewportPanel.hpp"

#include <imgui.h>
#include <crucible/Window.hpp>
#include <crucible/Input.hpp>
#include <crucible/Renderer.hpp>


static void orbitCamera(Camera &cam) {
    static vec3 camDirection = normalize(vec3(0.0f, 0.3f, 1.0f));
    static float camDistance = 10.0f;
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

void ViewportPanel::renderGrid() {
    int count = 10;

    for (int x = -count; x < count; x++) {
        for (int z = -count; z < count; z++) {

            vec3 xColor = z == 0 ? vec3(1.0f, 0.0f, 0.0f) : vec3(0.8f);
            vec3 zColor = x == 0 ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.8f);

            Renderer::debug.renderDebugLine(vec3((float)x, 0.0f, (float)z), vec3((float)x + 1.0f, 0.0f, (float)z), xColor);
            Renderer::debug.renderDebugLine(vec3((float)x, 0.0f, (float)z), vec3((float)x, 0.0f, (float)z + 1.0f), zColor);

            if (z == count-1) {
                Renderer::debug.renderDebugLine(vec3((float)x, 0.0f, (float)z + 1.0f), vec3((float)x + 1.0f, 0.0f, (float)z + 1.0f), vec3(0.8f));
            }
            if (x == count-1) {
                Renderer::debug.renderDebugLine(vec3((float)x + 1.0f, 0.0f, (float)z), vec3((float)x + 1.0f, 0.0f, (float)z + 1.0f), vec3(0.8f));
            }
        }
    }
}

ViewportPanel::ViewportPanel(EditorContext &context) : context(context) {
    orbitCamera(cam); // run once to set starting camera position

}

void ViewportPanel::renderContents() {
    Transform trans = Transform(vec3(), quaternion(), vec3(1.0f));

    //Renderer::render(context.model, trans);



    bool p_open = true;


    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("Viewport")) {

        float width = ImGui::GetContentRegionAvail().x;
        float height = ImGui::GetContentRegionAvail().y;

        vec2i rendererResolution = Renderer::getResolution();

        if (rendererResolution.x != (int)width || rendererResolution.y != (int)height) {
            Renderer::resize(width, height);
        }

        cam.dimensions = {width, height};

        context.scene.render();
        renderGrid();

        const Texture &t = Renderer::flushToTexture(cam);

        ImVec2 cpos = ImGui::GetCursorPos();
        cpos.x += 10.0f;
        cpos.y += 10.0f;

        ImGui::Image(ImTextureID((long long) t.getID()), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(0, 0, 0, 0));

        if (ImGui::IsItemHovered() || Window::getMouseGrabbed()) {
            orbitCamera(cam);
        }

        ImGui::SetCursorPos(cpos);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);


        ImGui::End();

    }
    ImGui::PopStyleVar();
}