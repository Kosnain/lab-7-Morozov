#define GLEW_DLL
#define GLFW_DLL

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GpuProgram.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Z / X — поворот узла
float rotNode = 0.0f;

// C / V — горизонтальное движение двух мешей
float moveCV = 0.0f;

// B / N — вертикальное движение нужного меша
float moveBN = 0.0f;

float minMoveCV = -0.7f;
float maxMoveCV = 0.7f;

float minMoveBN = -0.7f;
float maxMoveBN = 0.7f;

// Центр поворотного узла для Z / X
glm::vec3 pivotNode = glm::vec3(0.0f, 0.840645f, 1.230870f);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }

    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    glm::vec3 front;

    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float rotSpeed = 60.0f * deltaTime;
    float moveSpeed = 1.0f * deltaTime;

    // Z / X — поворот узла
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        rotNode -= rotSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        rotNode += rotSpeed;
    }

    // C / V — движение по горизонтальной оси Z
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        moveCV += moveSpeed;

        if (moveCV > maxMoveCV) {
            moveCV = maxMoveCV;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        moveCV -= moveSpeed;

        if (moveCV < minMoveCV) {
            moveCV = minMoveCV;
        }
    }

    // B / N — вертикальное движение нужного меша
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        moveBN -= moveSpeed;

        if (moveBN < minMoveBN) {
            moveBN = minMoveBN;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        moveBN += moveSpeed;

        if (moveBN > maxMoveBN) {
            moveBN = maxMoveBN;
        }
    }
}

int main() {
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "Lab 7 - Affine Transformations",
        NULL,
        NULL
    );

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GpuProgram shader("vertex.vert", "fragment.frag");
    Model ourModel("as.obj");

    std::cout << "=== LAB 7: AFFINE TRANSFORMATIONS ===" << std::endl;
    std::cout << "Camera: W A S D + mouse" << std::endl;
    std::cout << "Rotation node: Z / X" << std::endl;
    std::cout << "C / V: move BN mesh and mesh before it" << std::endl;
    std::cout << "B / N: move only needed mesh vertically" << std::endl;
    std::cout << "Meshes count: " << ourModel.getMeshesCount() << std::endl;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            800.0f / 600.0f,
            0.1f,
            100.0f
        );

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        shader.SetUniform("projection", glm::value_ptr(projection), false);
        shader.SetUniform("view", glm::value_ptr(view), false);

        shader.SetUniform("material.ambient", 0.2f, 0.3f, 0.8f);
        shader.SetUniform("material.diffuse", 0.4f, 0.6f, 1.0f);
        shader.SetUniform("material.specular", 0.8f, 0.8f, 0.8f);
        shader.SetUniform("material.shininess", 32.0f);

        shader.SetUniform("light.position", 2.0f, 5.0f, 3.0f);
        shader.SetUniform("light.ambient", 0.2f, 0.2f, 0.2f);
        shader.SetUniform("light.diffuse", 0.8f, 0.8f, 0.8f);
        shader.SetUniform("light.specular", 1.0f, 1.0f, 1.0f);

        shader.SetUniform("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // 0. Основание стоит
        glm::mat4 modelBase = glm::mat4(1.0f);

        // 1. Неподвижная часть стоит
        glm::mat4 modelCube001 = glm::mat4(1.0f);

        // Z / X — поворотный узел
        glm::mat4 rotationNode = glm::mat4(1.0f);

        rotationNode = glm::translate(rotationNode, pivotNode);

        rotationNode = glm::rotate(
            rotationNode,
            glm::radians(rotNode),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        rotationNode = glm::translate(rotationNode, -pivotNode);

        // Меш самого поворотного узла
        glm::mat4 modelZX = rotationNode;

        // C / V — горизонтальное движение
        glm::mat4 cvMove = glm::mat4(1.0f);

        cvMove = glm::translate(
            cvMove,
            glm::vec3(0.0f, 0.0f, -moveCV)
        );

        // Меш перед нужным:
        // наследует Z / X и двигается по C / V
        glm::mat4 modelCV = rotationNode * cvMove;

        // B / N — вертикальное движение
        glm::mat4 bnMove = glm::mat4(1.0f);

        bnMove = glm::translate(
            bnMove,
            glm::vec3(0.0f, moveBN, 0.0f)
        );

        // Нужный меш:
        // наследует Z / X, C / V и дополнительно двигается по B / N
        glm::mat4 modelBN = rotationNode * cvMove * bnMove;

        ourModel.Draw(shader, modelBase, modelCube001, modelZX, modelCV, modelBN);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}