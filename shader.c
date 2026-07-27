#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void loadFileToRAM(char** buffer, char* path)
{
    FILE* filePointer = fopen(path,"r");
    if(filePointer == NULL){ printf("Could not open %s\n",path); return;}
    fseek(filePointer,0,SEEK_END);
    long fileSize =  ftell(filePointer);
    rewind(filePointer);

    *buffer = malloc((fileSize + 1) * sizeof(char));
    fread(*buffer,sizeof(char),fileSize,filePointer);
    (*buffer)[fileSize] = '\0';
}

void buildShader(Shader* shader,const char* vertexPath,const char* fragmentPath)
{
    char* vertexShaderBuffer;
    loadFileToRAM(&vertexShaderBuffer,vertexPath);
    printf("%s",vertexShaderBuffer);

    char* fragmentShaderBuffer;
    loadFileToRAM(&fragmentShaderBuffer,fragmentPath);

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderBuffer, NULL);
    glCompileShader(vertex);

    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR::VERTEX::COMPILATION FAILED\n");
        printf("%s",infoLog);
    };

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderBuffer, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR::FRAGMENT::COMPILATION FAILED\n");
        printf("%s",infoLog);
    };

    shader->id = glCreateProgram();
    glAttachShader(shader->id, vertex);
    glAttachShader(shader->id, fragment);
    glLinkProgram(shader->id);
    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shader->id, 512, NULL, infoLog);
        printf("ERROR::SHADER::LINKING FAILED\n");
        printf("%s",infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void useShader(Shader* shader)
{
    glUseProgram(shader->id);
}

void setBool(Shader* shader, const char* name, bool value)
{
    glUniform1i(glGetUniformLocation(shader->id, name), (int)value);
}
void setInt(Shader* shader, const char* name, int value)
{
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}
void setFloat(Shader* shader, const char* name, float value)
{
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}