#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera() {
    updateDirection();
}

// Yaw y pitch se convierten de grados a radianes para calcular el frente con seno y coseno.
void Camera::updateDirection() {
    const float yawRad = glm::radians(yaw_);
    const float pitchRad = glm::radians(pitch_);

    glm::vec3 direction(
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)
    );

    // La normalización mantiene la dirección y establece longitud uno.
    front_ = glm::normalize(direction);
}

// WASD cambia la posición; Escape cierra la ventana.
void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Desplazamiento = rapidez por tiempo, independiente de los FPS.
    const float step = 0.45f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position_ += step * front_;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position_ -= step * front_;

    // El producto cruz entre frente y arriba da la dirección lateral.
    const glm::vec3 right = glm::normalize(glm::cross(front_, up_));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position_ -= step * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position_ += step * right;
}

// La diferencia entre las posiciones del cursor modifica yaw y pitch.
void Camera::processMouse(double x, double y) {
    if (firstMouse_) {
        lastX_ = static_cast<float>(x);
        lastY_ = static_cast<float>(y);
        firstMouse_ = false;
        return;
    }

    const float dx = (static_cast<float>(x) - lastX_) * 0.1f;
    const float dy = (lastY_ - static_cast<float>(y)) * 0.1f;
    lastX_ = static_cast<float>(x);
    lastY_ = static_cast<float>(y);

    yaw_ += dx;
    // Limito pitch para evitar que la cámara apunte exactamente en vertical.
    pitch_ = glm::clamp(pitch_ + dy, -89.0f, 89.0f);
    updateDirection();
}

// lookAt crea la matriz View con posición, objetivo y eje vertical.
glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}
