#include "Model.hpp"
#include "Shader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <iostream>
#include <map>
#include <algorithm>
#include <limits>
#include <cstddef>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Creo la textura 2D a partir de píxeles RGBA.
GLuint Model::createTexture(const unsigned char* bytes, int width, int height) {
    if (!bytes || width <= 0 || height <= 0) return 0;
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    // Los mipmaps ayudan cuando la textura aparece pequeña en pantalla.
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    return texture;
}

// Decodifico las imágenes incrustadas o externas usando stb_image.
GLuint Model::loadImage(const aiScene* scene, const std::string& gltfPath,
                        const aiString& reference) {
    const aiTexture* embedded = scene->GetEmbeddedTexture(reference.C_Str());
    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = nullptr;

    if (embedded && embedded->mHeight != 0) {
        width = static_cast<int>(embedded->mWidth);
        height = static_cast<int>(embedded->mHeight);
        std::vector<unsigned char> rgba(static_cast<size_t>(width) * height * 4);
        for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
            rgba[4*i]     = embedded->pcData[i].r;
            rgba[4*i + 1] = embedded->pcData[i].g;
            rgba[4*i + 2] = embedded->pcData[i].b;
            rgba[4*i + 3] = embedded->pcData[i].a;
        }
        return createTexture(rgba.data(), width, height);
    }

    if (embedded) {
        pixels = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(embedded->pcData),
            static_cast<int>(embedded->mWidth), &width, &height, &channels, 4);
    } else {
        auto file = std::filesystem::path(gltfPath).parent_path() / reference.C_Str();
        pixels = stbi_load(file.string().c_str(), &width, &height, &channels, 4);
        if (!pixels) std::cerr << "Textura no encontrada: " << file << '\n';
    }

    GLuint texture = createTexture(pixels, width, height);
    if (pixels) stbi_image_free(pixels);
    return texture;
}

// Assimp lee las mallas y los materiales del archivo glTF.
bool Model::load(const std::string& path, float maxSize) {
    destroy();
    Assimp::Importer importer;
    // Triangulo las caras, preparo los vértices y genero normales si hacen falta.
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices | aiProcess_GenNormals);
    if (!scene || !scene->mRootNode || !scene->mNumMeshes) {
        std::cerr << "No se pudo cargar " << path << ": " << importer.GetErrorString() << '\n';
        return false;
    }

    // La textura blanca sirve como respaldo cuando falta un mapa de color.
    const unsigned char white[] = {255, 255, 255, 255};
    GLuint whiteTexture = createTexture(white, 1, 1);
    if (!whiteTexture) return false;
    textures_.push_back(whiteTexture);

    std::map<std::string, GLuint> cache;
    materials_.resize(scene->mNumMaterials);
    stbi_set_flip_vertically_on_load(true);
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        aiMaterial* assimpMaterial = scene->mMaterials[i];
        Material material;
        material.texture = whiteTexture;
        aiColor4D color(1, 1, 1, 1);
        if (assimpMaterial->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS)
            assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        material.color = glm::vec4(color.r, color.g, color.b, color.a);

        aiString reference;
        if (assimpMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &reference) == AI_SUCCESS ||
            assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &reference) == AI_SUCCESS) {
            std::string key = reference.C_Str();
            if (!cache.count(key)) {
                cache[key] = loadImage(scene, path, reference);
                if (cache[key]) textures_.push_back(cache[key]);
            }
            if (cache[key]) material.texture = cache[key];
        }
        materials_[i] = material;
    }
    if (materials_.empty()) materials_.push_back(Material{whiteTexture, glm::vec4(1.0f)});

    // Calculo la caja delimitadora usando el mínimo y máximo de cada eje.
    glm::vec3 minPoint(std::numeric_limits<float>::max());
    glm::vec3 maxPoint(std::numeric_limits<float>::lowest());
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            const auto& p = mesh->mVertices[v];
            glm::vec3 pos(p.x, p.y, p.z);
            minPoint = glm::min(minPoint, pos);
            maxPoint = glm::max(maxPoint, pos);
        }
    }
    // Centro = (mínimo + máximo)/2. La escala depende de la dimensión más larga.
    glm::vec3 center = (minPoint + maxPoint) * 0.5f;
    glm::vec3 dims = maxPoint - minPoint;
    float biggest = std::max(dims.x, std::max(dims.y, dims.z));
    float scale = biggest > 0.000001f ? maxSize / biggest : 1.0f;

    // Preparo los vértices y los índices de los triángulos.
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        const unsigned int base = static_cast<unsigned int>(vertices.size());
        Part part{static_cast<unsigned int>(indices.size()), 0,
                  mesh->mMaterialIndex < materials_.size() ? mesh->mMaterialIndex : 0};
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            const auto& p = mesh->mVertices[v];
            Vertex vertex{};
            // Centro y escalo los vértices con (posición - centro) por escala.
            vertex.position = (glm::vec3(p.x, p.y, p.z) - center) * scale;
            if (mesh->HasTextureCoords(0)) {
                const auto& uv = mesh->mTextureCoords[0][v];
                vertex.uv = glm::vec2(uv.x, uv.y);
            }
            // La normal define la orientación de la superficie para calcular la luz.
            if (mesh->HasNormals()) {
                const auto& n = mesh->mNormals[v];
                vertex.normal = glm::vec3(n.x, n.y, n.z);
            } else {
                vertex.normal = glm::vec3(0, 1, 0);
            }
            vertices.push_back(vertex);
        }
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue;
            for (unsigned int k = 0; k < 3; ++k)
                // Sumo el desplazamiento base para unir los índices de varias mallas.
                indices.push_back(base + face.mIndices[k]);
        }
        part.count = static_cast<unsigned int>(indices.size()) - part.start;
        if (part.count) parts_.push_back(part);
    }

    if (indices.empty()) {
        destroy();
        std::cerr << "Modelo sin triangulos: " << path << '\n';
        return false;
    }

    // Creo los buffers: VAO para atributos, VBO para vértices y EBO para índices.
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    // Atributos 0, 1 y 2: posición XYZ, coordenadas UV y normal XYZ.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    std::cout << path << ": " << vertices.size() << " vertices, "
              << indices.size() / 3 << " triangulos\n";
    return true;
}

// Con MVP transformo el modelo; con glDrawElements dibujo cada parte.
void Model::draw(const Shader& shader, const glm::mat4& viewProjection,
                 const glm::mat4& transform, bool discardBlack) const {
    if (!loaded()) return;
    shader.use();
    // MVP = Projection por View por Model; uModel también transforma las normales.
    shader.setMat4("uMVP", viewProjection * transform);
    shader.setMat4("uModel", transform);
    shader.setInt("uDescartarNegro", discardBlack ? 1 : 0);
    glBindVertexArray(vao_);
    for (const Part& part : parts_) {
        const Material& material = materials_[part.material];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.texture);
        shader.setVec4("uBaseColor", material.color);
        // El EBO indica los índices que forman los triángulos.
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(part.count), GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(static_cast<size_t>(part.start) * sizeof(unsigned int)));
    }
    glBindVertexArray(0);
}

// Al cerrar libero texturas, VAO, VBO y EBO.
void Model::destroy() {
    for (GLuint texture : textures_) if (texture) glDeleteTextures(1, &texture);
    textures_.clear();
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
    vao_ = vbo_ = ebo_ = 0;
    parts_.clear();
    materials_.clear();
}
