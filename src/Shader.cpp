#include "Shader.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

// Leo los archivos GLSL desde la carpeta shaders.
static std::string readText(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "No se pudo abrir shader: " << path << '\n';
        return {};
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Creo el shader, le paso su código y compruebo la compilación.
GLuint Shader::compile(GLenum type, const std::string& source) {
    const GLuint shader = glCreateShader(type);
    const char* text = source.c_str();
    glShaderSource(shader, 1, &text, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Error al compilar shader:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Compilo el vertex y el fragment shader y los enlazo con glLinkProgram.
bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    destroy();
    const std::string vs = readText(vertexPath);
    const std::string fs = readText(fragmentPath);
    if (vs.empty() || fs.empty()) return false;

    const GLuint vertex = compile(GL_VERTEX_SHADER, vs);
    const GLuint fragment = compile(GL_FRAGMENT_SHADER, fs);
    if (!vertex || !fragment) {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vertex);
    glAttachShader(program_, fragment);
    glLinkProgram(program_);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096] = {};
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        std::cerr << "Error al enlazar programa:\n" << log << '\n';
        destroy();
        return false;
    }
    return true;
}

// Activo el programa antes de enviar sus datos y dibujar.
void Shader::use() const {
    glUseProgram(program_);
}

// Los uniforms conectan las matrices de C++ con el código GLSL.
void Shader::setMat4(const char* name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec3(const char* name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(value));
}
void Shader::setVec4(const char* name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(value));
}
void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program_, name), value);
}
// Libero el programa de OpenGL antes de cerrar el contexto.
void Shader::destroy() {
    if (program_) glDeleteProgram(program_);
    program_ = 0;
}
