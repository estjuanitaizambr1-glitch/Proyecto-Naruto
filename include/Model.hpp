#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Shader;
struct aiScene;
struct aiString;

// Cada objeto glTF tiene geometría, materiales, texturas y buffers propios.
class Model {
public:
    bool load(const std::string& path, float maxSize);
    void draw(const Shader& shader, const glm::mat4& viewProjection,
              const glm::mat4& transform, bool discardBlack) const;
    bool loaded() const { return vao_ != 0; }
    void destroy();

    Model() = default;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

private:
    // Un vértice contiene posición, coordenadas UV y normal para la iluminación.
    struct Vertex {
        glm::vec3 position{};
        glm::vec2 uv{};
        glm::vec3 normal{};
    };
    struct Part {
        unsigned int start, count, material;
    };
    struct Material {
        GLuint texture = 0;
        glm::vec4 color{1.0f};
    };

    // VAO configura los atributos; VBO guarda vértices y EBO guarda índices.
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    std::vector<Part> parts_;
    std::vector<Material> materials_;
    std::vector<GLuint> textures_;

    static GLuint createTexture(const unsigned char* bytes, int width, int height);
    static GLuint loadImage(const aiScene* scene, const std::string& gltfPath,
                            const aiString& reference);
};
