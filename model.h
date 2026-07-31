//
// Created by Nmsou on 31/07/2026.
//

#ifndef RENDERER_MODEL_H
#define RENDERER_MODEL_H

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

typedef struct
{
    char* path;
    Mesh* meshes;
    unsigned int numMeshes;
    char* dir; //?

}Model;

#define MAX_CACHED_TEXTURES 512
typedef struct {
    char path[256];
    unsigned int id;
} CachedTexture;

void drawMesh(Mesh* mesh, Shader* shader);

void setupMesh(Mesh* mesh);

unsigned int textureFromFile(const char* path, const char* directory);

Texture* loadMaterialTextures(struct aiMaterial* mat, enum aiTextureType type, const char* typeName, unsigned int* outCount, const char* directory);

unsigned int textureFromFileCached(const char* path, const char* directory);

Mesh processMesh(struct aiMesh *mesh, const struct aiScene *scene, Model* model);

void processNode(struct aiNode *node, const struct aiScene *scene, Model* model, unsigned int* meshIndex);

void loadModel(Model* model);

void drawModel(Model* model, Shader* shader);
#endif //RENDERER_MODEL_H
