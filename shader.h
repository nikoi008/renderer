//
// Created by Nmsou on 27/07/2026.
//

#ifndef RENDERER_SHADER_H
#define RENDERER_SHADER_H

#include <glad/glad.h>
#include <stdbool.h>

typedef struct
{
    unsigned int id;
}Shader;
void loadFileToRAM(char** buffer, const char* path);
void buildShader(Shader* shader,const char* vertexPath,const char* fragmentPath);
void useShader(Shader* shader);
void setBool(Shader* shader,const char* name,bool value);
void setInt(Shader* shader,const char* name,int value);
void setFloat(Shader* shader,const char* name,float value);

#endif //RENDERER_SHADER_H
