#pragma once
#include "Model.hpp"
#include "BezierCurve.hpp"
#include <glm/glm.hpp>

class Shader;

// La escena reúne Konoha, Naruto y el pájaro y administra sus transformaciones.
class Scene {
public:
    Scene();
    bool load();
    void draw(const Shader& shader, const glm::mat4& viewProjection, float timeSec) const;
    void destroy();

private:
    Model konoha_;
    Model bird_;
    Model naruto_;
    // La trayectoria se calcula con cuatro puntos de control.
    BezierCurve flight_;
};
