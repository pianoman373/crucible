#include <crucible/Util.hpp>
#include <crucible/Input.hpp>
#include <crucible/Window.hpp>

#include <math.h>

namespace Util {
    void updateSpaceCamera(Camera &cam, float speed) {
        static vec2 lastMousePos;
        Window::setMouseGrabbed(Input::isMouseButtonDown(1));

        if (Input::isMouseButtonDown(1)) {
            vec3 right = cam.getRight();
            float deltaSpeed = speed * Window::deltaTime();

            if (Input::isKeyDown(Input::KEY_A))
                cam.position = cam.position - right * deltaSpeed;

            if (Input::isKeyDown(Input::KEY_D))
                cam.position = cam.position + right * deltaSpeed;

            if (Input::isKeyDown(Input::KEY_W))
                cam.position = cam.position + cam.direction * deltaSpeed;

            if (Input::isKeyDown(Input::KEY_S))
                cam.position = cam.position - cam.direction * deltaSpeed;

            if (Input::isKeyDown(Input::KEY_R))
                cam.position = cam.position + cam.up * deltaSpeed;

            if (Input::isKeyDown(Input::KEY_F))
                cam.position = cam.position - cam.up * deltaSpeed;


            //rotation
            vec2 offset = Input::getCursorPos() - lastMousePos;
            float xOffset = -offset.x / 10.0f;
            float yOffset = offset.y / 10.0f;
            float zOffset = 0.0f;

            if (Input::isKeyDown(Input::KEY_Q)) {
                zOffset -= 45.0f * Window::deltaTime();
            }
            if (Input::isKeyDown(Input::KEY_E)) {
                zOffset += 45.0f * Window::deltaTime();
            }

            //vertical
            mat4 mat;
            mat = rotate(mat, right, -yOffset);
            vec4 vec = mat * vec4(cam.direction, 1.0f);
            cam.direction = normalize(vec3(vec));

            //horizontal
            mat = mat4();
            mat = rotate(mat, cam.up, xOffset);
            vec4 vecj = mat * vec4(cam.direction, 1.0f);
            cam.direction = normalize(vec3(vecj));

            mat = mat4();
            mat = rotate(mat, cam.direction, zOffset);
            vec4 veck = mat * vec4(right, 1.0f);
            right = normalize(vec3(veck));

            cam.up = normalize(cross(right, cam.direction));
        }
        lastMousePos = Input::getCursorPos();
    }

    float rand() {
        return (float)std::rand() / (float)RAND_MAX;
    }

    float rand(float min, float max) {
        return (rand() * (max - min)) + min;
    }
}