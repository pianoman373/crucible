#include <crucible/Shader.hpp>

#include <crucible/Resource.h>

#include <glad/glad.h>

#include <string>
#include <regex>

std::string str_replace( std::string const & in, std::string const & from, std::string const & to )
{
    return std::regex_replace( in, std::regex(from), to );
}

void loadLibraries(std::string &in) {
    //allow 5 levels of recursion
    for (int i = 0; i < 5; i++) {
        in = str_replace(in, "#include <lighting>", LOAD_RESOURCE(src_shaders_lighting_glsl).data());
    }
}


Shader::Shader() {

}

void Shader::load(std::string vertex, std::string fragment, std::string geometry) {
    loadLibraries(vertex);
    loadLibraries(fragment);

    const char* vShaderCode = vertex.c_str();
    const char* fShaderCode = fragment.c_str();
    const char* fGeometryCode = geometry.c_str();

    //shaders
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl << vertex << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl << fragment << std::endl;
    }

    unsigned int geometryShader = 0;

    if (!geometry.empty()) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &fGeometryCode, NULL);
        glCompileShader(geometryShader);
    }

    this->id = glCreateProgram();

    glAttachShader(this->id, vertexShader);
    glAttachShader(this->id, fragmentShader);
    if (!geometry.empty()) {
        glAttachShader(this->id, geometryShader);
    }

    glLinkProgram(this->id);

    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(this->id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::loadPostProcessing(std::string shader) {
    std::string fullShader = R"(
#version 330 core
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outColor1;
in vec2 fTexCoord;

)" + shader + R"(
void main()
{
    outColor = vec4(postProcess(fTexCoord), 1.0);
}
)";
    load(R"(
#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

out vec2 fTexCoord;

void main()
{
    fTexCoord = vTexCoord;
    gl_Position = vec4(vPosition, 1.0);
}
)", fullShader);

}



void Shader::bind() const {
    glUseProgram(this->id);
}

void Shader::uniformMat4(const std::string &location, const mat4 &mat) const {
    unsigned int transformLoc = glGetUniformLocation(this->id, location.c_str());

    float matrixArray[] = {
        mat.m00, mat.m10, mat.m20, mat.m30,
        mat.m01, mat.m11, mat.m21, mat.m31,
        mat.m02, mat.m12, mat.m22, mat.m32,
        mat.m03, mat.m13, mat.m23, mat.m33
    };
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, matrixArray);
}

void Shader::uniformVec3(const std::string &location, const vec3 &vec) const {
    unsigned int transformLoc = glGetUniformLocation(this->id, location.c_str());
    glUniform3f(transformLoc, vec.x, vec.y, vec.z);
}

void Shader::uniformVec4(const std::string &location, const vec4 &vec) const {
    unsigned int transformLoc = glGetUniformLocation(this->id, location.c_str());
    glUniform4f(transformLoc, vec.x, vec.y, vec.z, vec.w);
}

void Shader::uniformInt(const std::string &location, int value) const {
    unsigned int transformLoc = glGetUniformLocation(this->id, location.c_str());
    glUniform1i(transformLoc,value);
}

void Shader::uniformFloat(const std::string &location, float value) const {
    unsigned int transformLoc = glGetUniformLocation(this->id, location.c_str());
    glUniform1f(transformLoc,value);
}

void Shader::uniformBool(const std::string &location, bool value) const {
    unsigned int transformLoc = glGetUniformLocation(this->id, location.c_str());
    glUniform1i(transformLoc,value);
}
