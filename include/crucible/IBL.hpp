#pragma once

#include <crucible/Texture.hpp>

namespace IBL {
    void generateIBLmaps(const vec3 &position, Cubemap &irradiance, Cubemap &specular);
}