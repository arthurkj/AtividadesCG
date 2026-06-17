#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f)) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f) {
        Position = position;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = -90.0f;
        Pitch = 0.0f;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(int direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;

        if (direction == 0) Position += Front * velocity; 
        if (direction == 1) Position -= Front * velocity; 
        if (direction == 2) Position -= Right * velocity; 
        if (direction == 3) Position += Right * velocity; 
    }

    void ProcessMouseMovement(float xoffset, float yoffset) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

        updateCameraVectors();
    }

    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
    
};

struct Object3D {
    GLuint VAO;
    int nVertices;
	GLuint textureID;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    std::vector<glm::vec3> pathPoints; 
    int currentTarget = 0;             
    bool isMoving = false;
    float tParam = 0.0f;
};

struct Light {
    glm::vec3 position;
    float kl;
    float kq;
};

// Protótipos das funções
int loadSimpleOBJ(std::string filePath, int &nVertices, std::string &texturePath, glm::vec3 &ka, glm::vec3 &kd, glm::vec3 &ks);
void loadScene(std::string filePath, Camera &camera, std::vector<Object3D> &objects, std::vector<Light> &lights, float &fov, float &zNear, float &zFar);