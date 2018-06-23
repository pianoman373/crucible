#pragma once

#include <crucible/Texture.hpp>

namespace IBL {
    void generateIBLmaps(Cubemap &irradiance, Cubemap &specular);
}