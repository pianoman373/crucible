#pragma once

#include <crucible/Camera.hpp>

namespace Util {
    void updateSpaceCamera(Camera &cam, float speed=8.0f);

    float rand();

    float rand(float min, float max);
}