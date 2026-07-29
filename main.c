#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cglm/cglm.h>
#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 800


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
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed: y ranges bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= camera.mouseSensitivity;
    yoffset *= camera.mouseSensitivity;

    camera.yaw   += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

    updateCameraVectors(&camera);
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

void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "openglwindow", NULL, NULL);
    if (window == NULL)
    {
        printf("failed to create a window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    initCamera(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);

    Shader triShader;
    buildShader(&triShader,"vs.vs","fs.fs");


    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    vec3 cubePositions[] = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        {2.4f, -0.4f, -3.5f},
        {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},
        {1.5f, 2.0f, -2.5f},
        {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}
    };
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("migu.jpg", &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        printf("width %d height %d channels %d\n", width, height, nrChannels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("failed to load texture %s\n", stbi_failure_reason());
    }
    stbi_image_free(data);

    mat4 model;
    mat4 view;
    mat4 projection;

    while (!glfwWindowShouldClose(window))
    {
        cameraInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, texture);
        useShader(&triShader);


        vec3 center;
        glm_vec3_add(camera.cameraPos, camera.cameraFront, center);
        glm_lookat(camera.cameraPos, center, camera.cameraUp, view);

        glm_perspective(glm_rad(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, projection);

        setMat4(&triShader,"model",model);
        setMat4(&triShader,"view",view);
        setMat4(&triShader,"projection",projection);

        glBindVertexArray(VAO);
        for (int i = 0; i < 10; i++)
        {
            //mat4 model;
            glm_mat4_identity(model);
            glm_translate(model,cubePositions[i]);
            float angle = 20.0f * i;
            glm_rotate(model,(float)glfwGetTime() * glm_rad(angle),(vec3){1.0f,0.3f,0.5f});
            setMat4(&triShader,"model",model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

