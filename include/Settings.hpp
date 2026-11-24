#pragma once
#include <glm/glm.hpp>

// Aquí guardo los tamaños, las posiciones y las velocidades de la escena.
namespace Settings {
    inline constexpr int WINDOW_WIDTH = 1500;
    inline constexpr int WINDOW_HEIGHT = 1500;

    inline constexpr float KONOHA_SIZE = 1.6f;
    inline constexpr float BIRD_SIZE = 0.11f;
    inline constexpr float NARUTO_SIZE = 0.15f;

    // Posición definitiva de Naruto dentro de la aldea.
    inline const glm::vec3 NARUTO_POSITION(-0.391986f, -0.0674478f, 0.264913f);
    // Su rotación sobre Y es de 45 grados por segundo: una vuelta en 8 segundos.
    inline constexpr float NARUTO_ROTATION_DEG_PER_SEC = 45.0f;

    // El recorrido paramétrico del pájaro dura diez segundos.
    inline constexpr float BIRD_FLIGHT_DURATION = 10.0f;
    inline constexpr float BIRD_YAW_CORRECTION_DEG = 0.0f;

    // P0 y P3 son los extremos; P1 y P2 modifican la forma de la curva.
    inline const glm::vec3 BEZIER_P0(-0.48f, 0.10f, 0.28f);
    inline const glm::vec3 BEZIER_P1(-0.26f, 0.28f, 0.12f);
    inline const glm::vec3 BEZIER_P2( 0.19f, 0.30f,-0.12f);
    inline const glm::vec3 BEZIER_P3( 0.41f, 0.13f,-0.22f);
}
