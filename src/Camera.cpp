#include <crucible/Camera.hpp>
#include <crucible/Window.hpp>

Camera::Camera() {

}

Camera::Camera(vec3 position) {
    this->position = position;
}

vec3 Camera::getPosition() {
    return this->position;
}

vec3 Camera::getDirection() {
    return this->direction;
}

vec3 Camera::getRight() {
    return normalize(cross(this->direction, this->up));
}

vec3 Camera::getUp() {
    return normalize(cross(getRight(), this->direction));
}

void Camera::setPosition(const vec3 &position) {
    this->position = position;
}

void Camera::setDirection(const vec3 &direction) {
    this->direction = direction;
}

mat4 Camera::getView() {
    return LookAt(this->position, this->position + normalize(direction), getUp());
}

mat4 Camera::getProjection() {
    vec2 size = dimensions;

    if (this->orthographic) {
        return ::orthographic(-size.x/2.0f, size.x/2.0f, -size.y/2.0f, size.y/2.0f, -100.0f, 100.0f);
    }
    else {

        return perspective(70, size.x / size.y, 0.1f, 10000.0f);
    }
}
