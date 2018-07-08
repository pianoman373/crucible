#include <crucible/Resources.hpp>

#include <map>
#include <string>

static std::map<std::string, Texture> m_Textures;

namespace Resources {
    Texture getTexture(std::string path) {
        // if texture already exists, return that handle
        if (m_Textures.find(path) != m_Textures.end())
            return m_Textures[path];


        Texture texture;
        texture.load(path.c_str());

        std::cout << "loading texture: " << path << std::endl;
        m_Textures[path] = texture;

        return texture;
    }
}