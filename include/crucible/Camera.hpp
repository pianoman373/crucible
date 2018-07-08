#pragma once

#include <crucible/Math.hpp>

class Camera {
public:
    vec3 position = vec3(0.0f, 0.0f, 0.0f);
    vec3 direction = vec3(0.0f, 0.0f, -1.0f);
    vec3 up = vec3(0.0f, 1.0f, 0.0f);

    /** If true, no perspective will be used, and the output will be "2D" **/
    bool orthographic = false;

    /** Represents the screen resolution for use in calculating the aspect ratio for projection **/
    vec2 dimensions;

public:
    Camera();

    Camera(vec3 position);

    vec3 getPosition();

    vec3 getDirection();

    /**
     * Returns the vector that is perpendicular to the look vector and the up vector (a.k.a "right").
     */
    vec3 getRight();

    vec3 getUp();

    void setPosition(const vec3 &position);

    void setDirection(const vec3 &direction);

    mat4 getView();

    mat4 getProjection();
};
