#pragma once

#include <crucible/Path.hpp>
#include <crucible/Shader.hpp>
#include <crucible/Texture.hpp>
#include <crucible/AssimpFile.hpp>
#include <crucible/Material.hpp>

namespace Resources {
    Texture &getTexture(const Path &path, bool pixelated=false);

    Shader &getShader(const Path &vertexShader, const Path &fragmentShader);

    Shader &getShader(const Path &vertexShader, const Path &fragmentShader, const Path &geometryShader);

    AssimpFile &getAssimpFile(const Path &path);

    Material &getMaterial(const Path &path);
}