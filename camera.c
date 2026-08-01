//
// Created by Nmsou on 29/07/2026.
//

#include <cglm/cglm.h>
#include "camera.h"
#include <GLFW/glfw3.h>

Camera camera;
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

void updateCameraVectors(Camera *camera)
{
    vec3 front;
    front[0] = cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    front[1] = sinf(glm_rad(camera->pitch));
    front[2] = sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));

    glm_vec3_normalize_to(front, camera->cameraFront);

    glm_vec3_cross(camera->cameraFront, camera->worldUp, camera->cameraRight);
    glm_vec3_normalize(camera->cameraRight);

    glm_vec3_cross(camera->cameraRight, camera->cameraFront, camera->cameraUp);
    glm_vec3_normalize(camera->cameraUp);
}

void initCamera(Camera *camera, vec3 pos, vec3 up, float yaw, float pitch)
{
    glm_vec3_copy(pos, camera->cameraPos);
    glm_vec3_copy(up, camera->worldUp);
    camera->yaw = yaw;
    camera->pitch = pitch;
    camera->cameraSpeed = 0.05f;
    camera->mouseSensitivity = 0.1f;
    camera->fov = 45.0f;

    updateCameraVectors(camera);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        float xpos = (float)xposIn;
        float ypos = (float)yposIn;

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        xoffset *= camera.mouseSensitivity;
        yoffset *= camera.mouseSensitivity;

        camera.yaw += xoffset;
        camera.pitch += yoffset;

        if (camera.pitch > 89.0f) camera.pitch = 89.0f;
        if (camera.pitch < -89.0f) camera.pitch = -89.0f;

        updateCameraVectors(&camera);
    }
    else
    {
        firstMouse = true;
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.fov -= (float)yoffset;
    if (camera.fov < 1.0f)  camera.fov = 1.0f;
    if (camera.fov > 45.0f) camera.fov = 45.0f;
}

void cameraInput(GLFWwindow *window)
{
    vec3 offset;
    vec3 crossProduct;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        glm_vec3_scale(camera.cameraFront, camera.cameraSpeed, offset);
        glm_vec3_add(camera.cameraPos, offset, camera.cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm_vec3_scale(camera.cameraFront, camera.cameraSpeed, offset);
        glm_vec3_sub(camera.cameraPos, offset, camera.cameraPos);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        //cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        glm_vec3_cross(camera.cameraFront,camera.cameraUp,crossProduct);
        glm_vec3_normalize(crossProduct);
        glm_vec3_scale(crossProduct,camera.cameraSpeed,offset);
        glm_vec3_sub(camera.cameraPos,offset,camera.cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        //cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        glm_vec3_cross(camera.cameraFront,camera.cameraUp,crossProduct);
        glm_vec3_normalize(crossProduct);
        glm_vec3_scale(crossProduct,camera.cameraSpeed,offset);
        glm_vec3_add(camera.cameraPos,offset,camera.cameraPos);
    }
}

