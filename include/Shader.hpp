#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

// Esta clase carga dos shaders GLSL y los enlaza en un programa de OpenGL.
class Shader {
public:
    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;
    // Los uniform permiten enviar matrices y parámetros desde C++ a la GPU.
    void setMat4(const char* name, const glm::mat4& value) const;
    void setVec3(const char* name, const glm::vec3& value) const;
    void setVec4(const char* name, const glm::vec4& value) const;
    void setInt(const char* name, int value) const;
    void destroy();

    Shader() = default;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

private:
    GLuint program_ = 0;
    static GLuint compile(GLenum type, const std::string& source);
};
