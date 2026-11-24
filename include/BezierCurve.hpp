#pragma once
#include <glm/glm.hpp>

// La curva cúbica calcula la posición del pájaro y su dirección tangente.
class BezierCurve {
public:
    BezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    glm::vec3 position(float t) const;
    glm::vec3 tangent(float t) const;

private:
    glm::vec3 p0_, p1_, p2_, p3_;
};
