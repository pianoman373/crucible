#include <crucible/Resources.hpp>
#include <crucible/Util.hpp>

#include <map>

static std::map<unsigned int, Texture> m_Textures;

namespace Resources {
    Texture getTexture(std::string path) {
        unsigned int id = SID(path);

        // if texture already exists, return that handle
        if (m_Textures.find(id) != m_Textures.end())
            return m_Textures[id];


        Texture texture;
        texture.load(path.c_str());

        std::cout << "loading texture: " << path << std::endl;
        m_Textures[id] = texture;

        return texture;
    }
}