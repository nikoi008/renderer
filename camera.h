//
// Created by Nmsou on 29/07/2026.
//

#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H
#include <cglm/cglm.h>
#include <GLFW/glfw3.h>
#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 800
#include <stdbool.h>
typedef struct {
    vec3 cameraPos;
    vec3 cameraFront;
    vec3 cameraUp;
    vec3 cameraRight;
    vec3 worldUp;
    float yaw;
    float pitch;
    float cameraSpeed;
    float mouseSensitivity;
    float fov;
} Camera;
extern Camera camera;
extern float lastX;
extern float lastY;
extern bool firstMouse;

void updateCameraVectors(Camera *camera);

void initCamera(Camera *camera, vec3 pos, vec3 up, float yaw, float pitch);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void cameraInput(GLFWwindow *window);

#endif //RENDERER_CAMERA_H
