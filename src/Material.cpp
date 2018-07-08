#include <crucible/Material.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/Resources.hpp>
#include <crucible/Path.hpp>

#include <fstream>



Material::Material() {

}

void Material::setDefaultPBRUniforms() {
    this->shader = Renderer::standardShader;

    setUniformBool("albedoTextured", false);
    setUniformVec3("albedoColor", vec3(0.3f));

    setUniformBool("roughnessTextured", false);
    setUniformFloat("roughnessColor", 0.5f);

    setUniformBool("metallicTextured", false);
    setUniformFloat("metallicColor", 0.0f);

    setUniformBool("normalTextured", false);

    setUniformBool("aoTextured", false);

    setUniformBool("emissionTextured", false);
    setUniformFloat("emission", 0.0f);

    setUniformBool("invertRoughness", false);
    setUniformBool("roughnessMetallicAlpha", false);
}

void Material::fromJson(json j, std::string workingDirectory) {
    setDefaultPBRUniforms();
    json uniforms = j["uniforms"];

    int textures = 0;

    if (j["name"].is_string()) {
        name = j["name"];
    }

    for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
    {
        std::string key = it.key();
        json value = it.value();

        if (value.is_boolean()) {
            setUniformBool(key, value.get<bool>());
        }
        if (value.is_string()) {
            if (!value.get<std::string>().empty()) {

                setUniformTexture(key, Resources::getTexture(Path::getFullPath(workingDirectory, value.get<std::string>())), textures);
                textures++;
            }
        }
        if (value.is_array()) {
            if (value.size() == 3) {
                setUniformVec3(key, vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>()));
            }
        }
        if (value.is_number()) {
            setUniformFloat(key, value.get<float>());
        }
    }
}

json Material::toJson(std::string workingDirectory) {
    json ret;
    json uniforms;

    std::map<std::string, float> *floatUniforms = this->getFloatUniforms();
    for (auto it = floatUniforms->begin(); it != floatUniforms->end(); ++it)
    {
        //shader.uniformFloat(it->first, it->second);
        uniforms[it->first] = it->second;
    }

    //boolean
    std::map<std::string, bool> *boolUniforms = this->getBoolUniforms();
    for (auto it = boolUniforms->begin(); it != boolUniforms->end(); ++it)
    {
        //shader.uniformBool(it->first, it->second);
        uniforms[it->first] = it->second;
    }

    //vec3
    std::map<std::string, vec3> *vec3Uniforms = this->getVec3Uniforms();
    for (auto it = vec3Uniforms->begin(); it != vec3Uniforms->end(); ++it)
    {
        //shader.uniformVec3(it->first, it->second);
       uniforms[it->first] = {it->second.x, it->second.y, it->second.z};
    }

    //textures
    std::map<std::string, UniformTexture> *textures = this->getTextureUniforms();
    for (auto it = textures->begin(); it != textures->end(); ++it)
    {
//        UniformTexture uniform = it->second;
//        uniform.tex.bind(uniform.unit);
//        shader.uniformInt(it->first, uniform.unit);

        uniforms[it->first] = Path::getRelativePath(workingDirectory, it->second.tex.getFilepath());

//        std::cout << "working directory: " << workingDirectory << std::endl;
//        std::cout << "raw path: " << it->second.tex.getFilepath() << std::endl;
//        std::cout << "final path: " << Path::getRelativePath(workingDirectory, it->second.tex.getFilepath()) << std::endl;
    }

    ret["uniforms"] = uniforms;
    ret["name"] = name;

    return ret;
}

void Material::loadFile(std::string file) {
    Path::format(file);
    std::string wd = Path::getWorkingDirectory(file);

    json j;
    std::ifstream o(file);
    o >> j;

    fromJson(j, wd);
}

void Material::saveFile(std::string file) {
    std::ofstream o(file);
    o << std::setw(4) << toJson(Path::getWorkingDirectory(file)) << std::endl;
}

//<-----===== non normal =====----->//

void Material::setPBRUniforms(vec3 albedo, float roughness, float metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", false);
    setUniformVec3("albedoColor", albedo);
    setUniformBool("roughnessTextured", false);
    setUniformFloat("roughnessColor", roughness);
    setUniformBool("metallicTextured", false);
    setUniformFloat("metallicColor", metallic);
    setUniformBool("normalTextured", false);
}

void Material::setPBRUniforms(Texture albedo, float roughness, float metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", true);
    setUniformTexture("albedoTex", albedo, 0);
    setUniformBool("roughnessTextured", false);
    setUniformFloat("roughnessColor", roughness);
    setUniformBool("metallicTextured", false);
    setUniformFloat("metallicColor", metallic);
    setUniformBool("normalTextured", false);
}

void Material::setPBRUniforms(Texture albedo, Texture roughness, float metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", true);
    setUniformTexture("albedoTex", albedo, 0);
    setUniformBool("roughnessTextured", true);
    setUniformTexture("roughnessTex", roughness, 1);
    setUniformBool("metallicTextured", false);
    setUniformFloat("metallicColor", metallic);
    setUniformBool("normalTextured", false);
}

void Material::setPBRUniforms(Texture albedo, float roughness, Texture metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", true);
    setUniformTexture("albedoTex", albedo, 0);
    setUniformBool("roughnessTextured", false);
    setUniformFloat("roughnessColor", roughness);
    setUniformBool("metallicTextured", true);
    setUniformTexture("metallicTex", metallic, 2);
    setUniformBool("normalTextured", false);
}

void Material::setPBRUniforms(Texture albedo, Texture roughness, Texture metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", true);
    setUniformTexture("albedoTex", albedo, 0);
    setUniformBool("roughnessTextured", true);
    setUniformTexture("roughnessTex", roughness, 1);
    setUniformBool("metallicTextured", true);
    setUniformTexture("metallicTex", metallic, 2);
    setUniformBool("normalTextured", false);
}

void Material::setPBRUniforms(vec3 albedo, Texture roughness, float metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", false);
    setUniformVec3("albedoColor", albedo);
    setUniformBool("roughnessTextured", true);
    setUniformTexture("roughnessTex", roughness, 1);
    setUniformBool("metallicTextured", false);
    setUniformFloat("metallicColor", metallic);
    setUniformBool("normalTextured", false);
}

void Material::setPBRUniforms(vec3 albedo, float roughness, Texture metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", false);
    setUniformVec3("albedoColor", albedo);
    setUniformBool("roughnessTextured", false);
    setUniformFloat("roughnessColor", roughness);
    setUniformBool("metallicTextured", true);
    setUniformTexture("metallicTex", metallic, 2);
    setUniformBool("normalTextured", false);
    
}

void Material::setPBRUniforms(vec3 albedo, Texture roughness, Texture metallic) {
    setDefaultPBRUniforms();
    setUniformBool("albedoTextured", false);
    setUniformVec3("albedoColor", albedo);
    setUniformBool("roughnessTextured", true);
    setUniformTexture("roughnessTex", roughness, 1);
    setUniformBool("metallicTextured", true);
    setUniformTexture("metallicTex", metallic, 2);
    setUniformBool("normalTextured", false);
    setUniformFloat("emission", 0.0f);
}

//<-----===== normal =====----->//

void Material::setPBRUniforms(vec3 albedo, float roughness, float metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", false);
	setUniformVec3("albedoColor", albedo);
	setUniformBool("roughnessTextured", false);
	setUniformFloat("roughnessColor", roughness);
	setUniformBool("metallicTextured", false);
	setUniformFloat("metallicColor", metallic);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(Texture albedo, float roughness, float metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", true);
	setUniformTexture("albedoTex", albedo, 0);
	setUniformBool("roughnessTextured", false);
	setUniformFloat("roughnessColor", roughness);
	setUniformBool("metallicTextured", false);
	setUniformFloat("metallicColor", metallic);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(Texture albedo, Texture roughness, float metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", true);
	setUniformTexture("albedoTex", albedo, 0);
	setUniformBool("roughnessTextured", true);
	setUniformTexture("roughnessTex", roughness, 1);
	setUniformBool("metallicTextured", false);
	setUniformFloat("metallicColor", metallic);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(Texture albedo, float roughness, Texture metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", true);
	setUniformTexture("albedoTex", albedo, 0);
	setUniformBool("roughnessTextured", false);
	setUniformFloat("roughnessColor", roughness);
	setUniformBool("metallicTextured", true);
	setUniformTexture("metallicTex", metallic, 2);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(Texture albedo, Texture roughness, Texture metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", true);
	setUniformTexture("albedoTex", albedo, 0);
	setUniformBool("roughnessTextured", true);
	setUniformTexture("roughnessTex", roughness, 1);
	setUniformBool("metallicTextured", true);
	setUniformTexture("metallicTex", metallic, 2);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(vec3 albedo, Texture roughness, float metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", false);
	setUniformVec3("albedoColor", albedo);
	setUniformBool("roughnessTextured", true);
	setUniformTexture("roughnessTex", roughness, 1);
	setUniformBool("metallicTextured", false);
	setUniformFloat("metallicColor", metallic);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(vec3 albedo, float roughness, Texture metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", false);
	setUniformVec3("albedoColor", albedo);
	setUniformBool("roughnessTextured", false);
	setUniformFloat("roughnessColor", roughness);
	setUniformBool("metallicTextured", true);
	setUniformTexture("metallicTex", metallic, 2);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setPBRUniforms(vec3 albedo, Texture roughness, Texture metallic, Texture normal) {
    setDefaultPBRUniforms();
	setUniformBool("albedoTextured", false);
	setUniformVec3("albedoColor", albedo);
	setUniformBool("roughnessTextured", true);
	setUniformTexture("roughnessTex", roughness, 1);
	setUniformBool("metallicTextured", true);
	setUniformTexture("metallicTex", metallic, 2);
	setUniformTexture("normalTex", normal, 3);
	setUniformBool("normalTextured", true);
    setUniformFloat("emission", 0.0f);
}

void Material::setUniformTexture(std::string name, Texture value, unsigned int unit) {
    textures[name].tex = value;
    textures[name].unit = unit;
}

void Material::setUniformVec3(std::string name, vec3 value) {
    vec3Uniforms[name] = value;
}

void Material::setUniformFloat(std::string name, float value) {
    floatUniforms[name] = value;
}

void Material::setUniformBool(std::string name, bool value) {
    boolUniforms[name] = value;
}

vec3 *Material::getUniformVec3(std::string name) {
    return &vec3Uniforms[name];
}

float *Material::getUniformFloat(std::string name) {
    return &floatUniforms[name];
}

bool *Material::getUniformBool(std::string name) {
    return &boolUniforms[name];
}

Texture *Material::getUniformTexture(std::string name) {
    return &textures[name].tex;
}

void Material::setShader(Shader shader) {
    this->shader = shader;
}

Shader Material::getShader() {
    return shader;
}

std::map<std::string, UniformTexture> *Material::getTextureUniforms() {
    return &textures;
}

std::map<std::string, vec3> *Material::getVec3Uniforms() {
    return &vec3Uniforms;
}

std::map<std::string, float> *Material::getFloatUniforms() {
    return &floatUniforms;
}

std::map<std::string, bool> *Material::getBoolUniforms() {
    return &boolUniforms;
}

void Material::bindUniforms() {
    //float
    std::map<std::string, float> *floatUniforms = this->getFloatUniforms();
    for (auto it = floatUniforms->begin(); it != floatUniforms->end(); ++it)
    {
        shader.uniformFloat(it->first, it->second);
    }

    //boolean
    std::map<std::string, bool> *boolUniforms = this->getBoolUniforms();
    for (auto it = boolUniforms->begin(); it != boolUniforms->end(); ++it)
    {
        shader.uniformBool(it->first, it->second);
    }

    //vec3
    std::map<std::string, vec3> *vec3Uniforms = this->getVec3Uniforms();
    for (auto it = vec3Uniforms->begin(); it != vec3Uniforms->end(); ++it)
    {
        shader.uniformVec3(it->first, it->second);
    }

    //textures
    std::map<std::string, UniformTexture> *textures = this->getTextureUniforms();
    for (auto it = textures->begin(); it != textures->end(); ++it)
    {
        UniformTexture uniform = it->second;
        uniform.tex.bind(uniform.unit);
        shader.uniformInt(it->first, uniform.unit);
    }
}
