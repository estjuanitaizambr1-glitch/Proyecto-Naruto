#include "BezierCurve.hpp"

// Guardo los cuatro puntos que definen la trayectoria.
BezierCurve::BezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
    : p0_(p0), p1_(p1), p2_(p2), p3_(p3) {}

// B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t)t^2 P2 + t^3 P3.
glm::vec3 BezierCurve::position(float t) const {
    const float s = 1.0f - t;

    return s*s*s*p0_ + 3.0f*s*s*t*p1_ +
           3.0f*s*t*t*p2_ + t*t*t*p3_;
}

// La derivada B'(t) describe la dirección de avance sobre la curva.
glm::vec3 BezierCurve::tangent(float t) const {
    const float s = 1.0f - t;

    return 3.0f*s*s*(p1_ - p0_) + 6.0f*s*t*(p2_ - p1_) +
           3.0f*t*t*(p3_ - p2_);
}
