#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "shader.h"

const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 1.0);\n"
"    ourColor = aColor;\n"
"}\0";

const char *fragmentShaderSource =
"#version 330 core\n"
"in vec3 ourColor;\n"
"\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(ourColor, 1.0);\n"
"}\0";
void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{

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
        printf("Failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);


    Shader triShader;
    buildShader(&triShader,"vs.vs","fs.fs");


    float vertices[] = {
        // positions           // colours
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,    0.0f, 0.0f, 1.0f
    };

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6 * sizeof(float),(void*)(3 * sizeof(float))); //colour attributs
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        useShader(&triShader);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}