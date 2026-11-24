#include "Scene.hpp"
#include "Shader.hpp"
#include "Settings.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>

// La curva del pájaro se inicializa con los puntos de Settings.
Scene::Scene()
    : flight_(Settings::BEZIER_P0, Settings::BEZIER_P1,
              Settings::BEZIER_P2, Settings::BEZIER_P3) {}

// Cargo los tres glTF de forma independiente con sus texturas.
bool Scene::load() {
    if (!konoha_.load("konoha/scene.gltf", Settings::KONOHA_SIZE)) return false;
    if (!naruto_.load("naruto/scene.gltf", Settings::NARUTO_SIZE)) return false;

    const bool birdFiles =
        std::filesystem::exists("pajaro/scene.gltf") &&
        std::filesystem::exists("pajaro/scene.bin") &&
        std::filesystem::exists("pajaro/textures/MI_ITM_DetonatingClayTwinBirds2_baseColor.jpeg");

    if (!birdFiles) {
        std::cerr << "Faltan archivos en pajaro/: revisa scene.gltf, scene.bin y textures/.\n";
    } else if (!bird_.load("pajaro/scene.gltf", Settings::BIRD_SIZE)) {
        std::cerr << "No se pudo cargar el pajaro; revisa sus archivos glTF.\n";
    }
    return true;
}

// Cada objeto usa una matriz Model distinta y comparte la misma cámara.
void Scene::draw(const Shader& shader, const glm::mat4& viewProjection, float timeSec) const {
    konoha_.draw(shader, viewProjection, glm::mat4(1.0f), true);

    if (bird_.loaded()) {
        // t recorre [0,1) cada diez segundos para repetir el vuelo.
        const float t = std::fmod(timeSec / Settings::BIRD_FLIGHT_DURATION, 1.0f);
        const glm::vec3 position = flight_.position(t);
        // La derivada de Bézier permite conocer la orientación del movimiento.
        const glm::vec3 tangent = flight_.tangent(t);

        if (glm::dot(tangent, tangent) > 1e-10f) {
            const glm::vec3 direction = glm::normalize(tangent);
            // atan2 convierte las componentes de la tangente en ángulos de rotación.
            const float yaw = std::atan2(direction.x, direction.z);
            const float pitch = -std::atan2(
                direction.y, glm::length(glm::vec2(direction.x, direction.z))
            );

            // Traslado al pájaro al punto Bézier y luego oriento el modelo sobre X e Y.
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
            transform = glm::rotate(transform, yaw, glm::vec3(0, 1, 0));
            transform = glm::rotate(transform, pitch, glm::vec3(1, 0, 0));
            transform = glm::rotate(transform,
                glm::radians(Settings::BIRD_YAW_CORRECTION_DEG), glm::vec3(0, 1, 0));
            bird_.draw(shader, viewProjection, transform, false);
        }
    }

    // Para Naruto aplico θ = velocidad angular por tiempo.
    const float angleDegrees = Settings::NARUTO_ROTATION_DEG_PER_SEC * timeSec;
    const float angleRadians = glm::radians(angleDegrees);
    // La transformación T × Ry lo mantiene en el sitio mientras gira sobre Y.
    glm::mat4 narutoTransform = glm::translate(glm::mat4(1.0f), Settings::NARUTO_POSITION);
    narutoTransform = glm::rotate(narutoTransform, angleRadians, glm::vec3(0, 1, 0));
    naruto_.draw(shader, viewProjection, narutoTransform, false);
}

// Libero los recursos gráficos de los tres modelos.
void Scene::destroy() {
    bird_.destroy();
    naruto_.destroy();
    konoha_.destroy();
}
