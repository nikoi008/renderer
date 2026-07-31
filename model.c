//
// Created by Nmsou on 31/07/2026.
//
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cglm/cglm.h>
#include "shader.h"
#include "stb_image.h"
#include "model.h"

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
        snprintf(buffer,sizeof(buffer),"%s%s",name,number);
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


unsigned int textureFromFile(const char* path, const char* directory)
{
    char fullPath[512];
    if (directory != NULL && directory[0] != '\0')
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, path);
    else
        snprintf(fullPath, sizeof(fullPath), "%s", path);
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
        textures[i].id = textureFromFileCached(string.data, directory);
        //textures[i].id = textureFromFile(string.data, directory);
        textures[i].type = strdup(typeName);
    }

    *outCount = count;
    return textures;
}



CachedTexture textureCache[MAX_CACHED_TEXTURES];
int textureCacheCount = 0;

unsigned int textureFromFileCached(const char* path, const char* directory)
{
    char fullPath[512];
    if (directory != NULL && directory[0] != '\0')
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, path);
    else
        snprintf(fullPath, sizeof(fullPath), "%s", path);

    for (int i = 0; i < textureCacheCount; i++)
    {
        if (strcmp(textureCache[i].path, fullPath) == 0)
            return textureCache[i].id;
    }

    unsigned int id = textureFromFile(path, directory);

    if (textureCacheCount < MAX_CACHED_TEXTURES)
    {
        strncpy(textureCache[textureCacheCount].path, fullPath, sizeof(textureCache[textureCacheCount].path) - 1);
        textureCache[textureCacheCount].id = id;
        textureCacheCount++;
    }

    return id;
}
Mesh processMesh(struct aiMesh *mesh, const struct aiScene *scene, Model* model)
{
    printf("processMesh: start, numVertices=%u, numFaces=%u\n", mesh->mNumVertices, mesh->mNumFaces);
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

    Mesh newMesh = (Mesh){vertices, numVertices, textures, numTextures, indices, numIndices};
    setupMesh(&newMesh);
    return newMesh;
}
void processNode(struct aiNode *node, const struct aiScene *scene, Model* model, unsigned int* meshIndex)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes[*meshIndex] = processMesh(mesh, scene, model);
        (*meshIndex)++;
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model, meshIndex);
    }
}
void loadModel(Model* model)
{
    const struct aiScene* scene = aiImportFile(model->path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
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
    unsigned int meshIndex = 0;
    processNode(scene->mRootNode, scene, model, &meshIndex);
    aiReleaseImport(scene);
}
void drawModel(Model* model, Shader* shader)
{
    for (int i = 0; i < model->numMeshes; i++)
    {
        drawMesh(&model->meshes[i],shader);
    }
}