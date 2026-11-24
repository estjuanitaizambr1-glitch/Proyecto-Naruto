#pragma once
#include <glm/glm.hpp>

struct GLFWwindow;

// La cámara guarda su posición y los ángulos que controlan la vista.
class Camera {
public:
    Camera();
    // El teclado mueve la cámara usando el tiempo entre fotogramas.
    void processKeyboard(GLFWwindow* window, float deltaTime);
    void processMouse(double x, double y);
    glm::mat4 viewMatrix() const;

private:
    // Convierto yaw y pitch en un vector frontal de longitud uno.
    void updateDirection();
    glm::vec3 position_{-0.634021f, -0.0514308f, 0.632884f};
    glm::vec3 front_{0.0f, 0.0f, -1.0f};
    glm::vec3 up_{0.0f, 1.0f, 0.0f};
    float yaw_ = -46.0999f;
    float pitch_ = -7.60001f;
    float lastX_ = 750.0f, lastY_ = 750.0f;
    bool firstMouse_ = true;
};
