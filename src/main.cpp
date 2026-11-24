#include "Camera.hpp"
#include "Shader.hpp"
#include "Scene.hpp"
#include "Settings.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// GLFW envía las coordenadas del cursor a la clase Camera.
static void mouseCallback(GLFWwindow* window, double x, double y) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera) camera->processMouse(x, y);
}

// Inicio la ventana y el contexto OpenGL 4.1 con GLFW y GLAD.
int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(
        Settings::WINDOW_WIDTH, Settings::WINDOW_HEIGHT, "Konoha", nullptr, nullptr
    );
    if (!window) {
        std::cerr << "No se pudo crear la ventana GLFW.\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "No se pudieron cargar las funciones de OpenGL.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // La prueba de profundidad oculta las superficies que están detrás.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

    // Preparo la cámara, la escena y el programa de shaders.
    Camera camera;
    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader shader;
    Scene scene;
    int result = 0;

    // El vertex shader y el fragment shader se compilan antes de cargar la escena.
    if (!shader.load("shaders/model.vert", "shaders/model.frag")) {
        result = 1;
    } else if (!scene.load()) {
        result = 1;
    } else {
        shader.use();
        shader.setInt("uTexture", 0);
        shader.setVec3("uLightDir", glm::vec3(-0.4f, 0.85f, 0.45f));
        shader.setVec3("uPointPos", glm::vec3(0.0f, 0.10f, 0.0f));

        float lastFrame = 0.0f;
        // El render loop actualiza tiempo, entradas, matrices y dibujo en cada fotograma.
        while (!glfwWindowShouldClose(window)) {
            const float now = static_cast<float>(glfwGetTime());
            const float deltaTime = now - lastFrame;
            lastFrame = now;

            // WASD y Escape controlan la cámara durante la ejecución.
            camera.processKeyboard(window, deltaTime);

            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            if (width == 0 || height == 0) {
                glfwPollEvents();
                continue;
            }
            glViewport(0, 0, width, height);
            // Limpio el color y la profundidad antes de dibujar el siguiente fotograma.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Projection establece la perspectiva; View depende de la orientación de la cámara.
            const glm::mat4 projection = glm::perspective(
                glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.0f
            );
            const glm::mat4 viewProjection = projection * camera.viewMatrix();
            // La escena dibuja Konoha, Naruto y el pájaro con transformaciones independientes.
            scene.draw(shader, viewProjection, now);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // Al finalizar libero modelos, programa de shaders y ventana GLFW.
    scene.destroy();
    shader.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
