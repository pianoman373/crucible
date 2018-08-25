#include <crucible/Camera.hpp>
#include <crucible/Window.hpp>

Camera::Camera() {

}

Camera::Camera(vec3 position) {
    this->position = position;
}

const vec3 &Camera::getPosition() const {
    return this->position;
}

const vec3 &Camera::getDirection() const {
    return this->direction;
}

vec3 Camera::getRight() const {
    return normalize(cross(this->direction, this->up));
}

vec3 Camera::getUp() const {
    return normalize(cross(getRight(), this->direction));
}

void Camera::setPosition(const vec3 &position) {
    this->position = position;
}

void Camera::setDirection(const vec3 &direction) {
    this->direction = direction;
}

mat4 Camera::getView() const {
    return LookAt(this->position, this->position + normalize(direction), getUp());
}

mat4 Camera::getProjection() const {
    vec2 size = dimensions;

    if (this->orthographic) {
        return ::orthographic(-size.x/2.0f, size.x/2.0f, -size.y/2.0f, size.y/2.0f, -100.0f, 100.0f);
    }
    else {

        return perspective(fov, size.x / size.y, 0.1f, 10000.0f);
    }
}