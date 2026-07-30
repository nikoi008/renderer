#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cglm/cglm.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "camera.h"

typedef struct
{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec3 tangent;
    vec3 bitangent;
}Vertex;

typedef struct
{
    int id;
    char* type;
}Texture;

typedef struct
{
    Vertex* vertices;
    unsigned int numVertices;
    Texture* textures;
    unsigned int numTextures;
    unsigned int* indices;
    unsigned int numIndices;
    unsigned int VAO,VBO,EBO;
}Mesh;
void drawMesh(Mesh* mesh, Shader* shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for(unsigned int i = 0; i < mesh->numTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        char number[16];
        char* name = mesh->textures[i].type;
        if (strcmp(name, "texture_diffuse") == 0)
        {
            //number = std::to_string(diffuseNr++);
            itoa(diffuseNr,number,10);
            diffuseNr++;

        }
        else if(strcmp(name, "texture_specular") == 0)
        {
            itoa(specularNr,number,10);
            specularNr++;
        }

        char buffer[128];
        snprintf(buffer,sizeof(buffer),"material.%s%s",name,number);
        setInt(shader,buffer,i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
void setupMesh(Mesh* mesh)
{
    glGenVertexArrays(1,&mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Vertex),&mesh->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

typedef struct
{
    char* path;
    Mesh* meshes;
    unsigned int numMeshes;
    char* dir; //?

}Model;
unsigned int textureFromFile(const char* path, const char* directory)
{
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, path);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(fullPath, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        printf("Texture failed to load at path: %s\n", fullPath);
    }

    stbi_image_free(data);

    return textureID;
}
Texture* loadMaterialTextures(struct aiMaterial* mat, enum aiTextureType type, const char* typeName, unsigned int* outCount, const char* directory)
{
    unsigned int count = aiGetMaterialTextureCount(mat, type);
    Texture* textures = malloc(count * sizeof(Texture));

    for (unsigned int i = 0; i < count; i++)
    {
        struct aiString string;
        aiGetMaterialTexture(mat, type, i, &string, NULL, NULL, NULL, NULL, NULL, NULL);

        textures[i].id = textureFromFile(string.data, directory);
        textures[i].type = strdup(typeName);
    }

    *outCount = count;
    return textures;
}
Mesh processMesh(struct aiMesh *mesh, const struct aiScene *scene,Model* model)
{
    unsigned int numVertices = mesh->mNumVertices;
    Vertex* vertices = malloc(numVertices * sizeof(Vertex));

    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vec3 vec;
        vec[0] = mesh->mVertices[i].x;
        vec[1] = mesh->mVertices[i].y;
        vec[2] = mesh->mVertices[i].z;
        vertex.position[0] = vec[0];
        vertex.position[1] = vec[1];
        vertex.position[2] = vec[2];

        if (mesh->mNormals != NULL)
        {
            vec[0] = mesh->mNormals[i].x;
            vec[1] = mesh->mNormals[i].y;
            vec[2]= mesh->mNormals[i].z;

            vertex.normal[0] = vec[0];
            vertex.normal[1] = vec[1];
            vertex.normal[2] = vec[2];
        }

        if (mesh->mTextureCoords[0])
        {
            vec2 vector;
            vector[0] = mesh->mTextureCoords[0][i].x;
            vector[1] = mesh->mTextureCoords[0][i].y;
            vertex.texCoords[0] = vector[0];
            vertex.texCoords[1] = vector[1];

            vec[0] = mesh->mTangents[i].x;
            vec[1] = mesh->mTangents[i].y;
            vec[2] = mesh->mTangents[i].z;
            vertex.tangent[0] = vec[0];
            vertex.tangent[1] = vec[1];
            vertex.tangent[2] = vec[2];
            // bitangent
            vec[0] = mesh->mBitangents[i].x;
            vec[1] = mesh->mBitangents[i].y;
            vec[2] = mesh->mBitangents[i].z;
            vertex.bitangent[0] = vec[0];
            vertex.bitangent[1] = vec[1];
            vertex.bitangent[2] = vec[2];
        }
        else
        {
            vertex.texCoords[0] = 0.0f;
            vertex.texCoords[1] = 0.0f;
        }

        vertices[i] = vertex;
    }

    unsigned int numIndices = mesh->mNumFaces * 3;
    unsigned int* indices = malloc(numIndices * sizeof(unsigned int));
    unsigned int indicesCount = 0;
    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        struct aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices;j++)
        {
            indices[indicesCount] = face.mIndices[j];
            indicesCount++;
        }
    }

    struct aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    unsigned int diffuseCount, specularCount, normalCount, heightCount;
    Texture* diffuseMaps = loadMaterialTextures(material,aiTextureType_DIFFUSE,"texture_diffuse",&diffuseCount,model->dir);
    Texture* specularMaps = loadMaterialTextures(material,aiTextureType_SPECULAR,"texture_specular",&specularCount,model->dir);
    Texture* normalMaps = loadMaterialTextures(material,aiTextureType_HEIGHT,"texture_normal", &normalCount,model->dir);
    Texture* heightMaps = loadMaterialTextures(material,aiTextureType_AMBIENT,"texture_height",&heightCount,model->dir);

    unsigned int numTextures = diffuseCount + specularCount + normalCount + heightCount;
    Texture* textures =malloc(numTextures * sizeof(Texture));


    unsigned int offset = 0;
    memcpy(textures+offset,diffuseMaps,diffuseCount * sizeof(Texture));
    offset += diffuseCount;
    memcpy(textures+offset,specularMaps,specularCount * sizeof(Texture));
    offset += specularCount;
    memcpy(textures+offset,normalMaps,normalCount * sizeof(Texture));
    offset += normalCount;
    memcpy(textures+offset,heightMaps,heightCount * sizeof(Texture));

    free(diffuseMaps); free(specularMaps); free(normalMaps); free(heightMaps);

    return (Mesh){vertices,numVertices, textures, numTextures,indices,numIndices};
}
void processNode(struct aiNode *node, const struct aiScene *scene,Model* model)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        //Model->meshes[i] = processMesh(mesh, scene);
        model->meshes[i] = processMesh(mesh,scene,model);
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene,model);
    }
}
void loadModel(Model* model)
{
    const struct aiScene* scene = aiImportFile(model->path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("very bad assimp error: %s\n", aiGetErrorString());
        return;
    }

    char* lastSlash = strrchr(model->path, '/');
    if (lastSlash != NULL)
    {
        size_t dirLength = lastSlash - model->path;
        model->dir = malloc(dirLength + 1);
        strncpy(model->dir, model->path, dirLength);
        model->dir[dirLength] = '\0';
    }
    else
    {
        model->dir = malloc(1);
        model->dir[0] = '\0';
    }

    model->numMeshes = scene->mNumMeshes;
    model->meshes = malloc(sizeof(Mesh) * model->numMeshes);
    processNode(scene->mRootNode, scene, model);
    aiReleaseImport(scene);
}
void drawModel(Model* model, Shader* shader)
{
    for (int i = 0; i < model->numMeshes; i++)
    {
        drawMesh(&model->meshes[i],shader);
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
    Model backpack;
    backpack.path = "backpack.obj";
    loadModel(&backpack);
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

        //setMat4(&triShader,"model",model);
        setMat4(&triShader,"view",view);
        setMat4(&triShader,"projection",projection);


        mat4 backpackModel;
        glm_mat4_identity(backpackModel);
        setMat4(&triShader, "model", backpackModel);
        drawModel(&backpack, &triShader);


        glBindVertexArray(VAO);
        /*for (int i = 0; i < 10; i++)
        {
            //mat4 model;
            glm_mat4_identity(model);
            glm_translate(model,cubePositions[i]);
            float angle = 20.0f * i;
            glm_rotate(model,(float)glfwGetTime() * glm_rad(angle),(vec3){1.0f,0.3f,0.5f});
            //setMat4(&triShader,"model",model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/


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

