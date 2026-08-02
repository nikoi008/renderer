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
    #include "model.h"
    #define NK_INCLUDE_FIXED_TYPES
    #define NK_INCLUDE_STANDARD_IO
    #define NK_INCLUDE_STANDARD_VARARGS
    #define NK_INCLUDE_DEFAULT_ALLOCATOR
    #define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    #define NK_INCLUDE_FONT_BAKING
    #define NK_INCLUDE_DEFAULT_FONT
    #define NK_IMPLEMENTATION
    #define NK_GLFW_GL3_IMPLEMENTATION
    #include "nuklear.h"
    #include "nuklear_glfw_gl3.h"
    #include <windows.h>

void freeMesh(Mesh* mesh)
{
    free(mesh->vertices);
    free(mesh->indices);

    for (int i = 0; i < mesh->numTextures; i++)
        free(mesh->textures[i].type);
    free(mesh->textures);

    glDeleteVertexArrays(1,&mesh->VAO);
    glDeleteBuffers(1,&mesh->VBO);
    glDeleteBuffers(1,&mesh->EBO);
}

void freeModel(Model* model)
{
    for (int i = 0; i < model->numMeshes; i++)
        freeMesh(&model->meshes[i]);

    free(model->meshes);
    free(model->dir);

    model->meshes = NULL;
    model->numMeshes = 0;
    model->dir = NULL;
}
typedef struct
{
    vec3 rotation;
    vec3 scale;
    vec3 position;
}transformation;
/*
void transformModel(Shader* shader,transformation t)
{
    mat4 trans;
    glm_mat4_identity(trans);
    glm_rotate(trans, glm_rad(90.0f), t.rotation);
    glm_scale(trans, t.scale);
    unsigned int transformLoc = glGetUniformLocation(shader->id,"transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE,*trans);

}
*/
void transformModel(Shader* shader, transformation t)
{
    mat4 trans;
    glm_mat4_identity(trans);
    glm_translate(trans, t.position);
    glm_rotate(trans,glm_rad(t.rotation[0]),(vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(trans,glm_rad(t.rotation[1]),(vec3){0.0f, 1.0f, 0.0f});
    glm_rotate(trans,glm_rad(t.rotation[2]),(vec3){0.0f, 0.0f, 1.0f});
    glm_scale(trans,t.scale);

    glm_translate(trans,t.position);
    setMat4(shader,"transform",trans);
}

void resizeFont(struct nk_glfw* glfw, struct nk_context* ctx, float scale)
{
    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(glfw, &atlas);

    struct nk_font_config config = nk_font_config(0);
    struct nk_font *font = nk_font_atlas_add_default(atlas,13 * scale,&config);

    nk_glfw3_font_stash_end(glfw);

    nk_style_set_font(ctx, &font->handle);
}
    void nuklearFrame(struct nk_context* ctx,struct nk_colorf* bg, GLFWwindow* window, struct nk_glfw* glfw, Model* model,Shader* shader)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if (width > 0 && height > 0)
    {
        float scaleX = (float)width / 800;
        float scaleY = (float)height / 600;
        static float oldScaleX = 1.0f;
        static float oldScaleY = 1.0f;

        if (scaleX != oldScaleX || scaleY != oldScaleY)
        {
            struct nk_rect bounds = nk_rect(0, 0, 230 * scaleX, 600 * scaleY);
            nk_window_set_bounds(ctx, "Config", bounds);
            resizeFont(glfw,ctx,scaleY);
            oldScaleX = scaleX;
            oldScaleY = scaleY;
        }

        if (nk_begin(ctx, "Config", nk_rect(0, 0, 230 * scaleX, 600 * scaleY),NK_WINDOW_BORDER|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE|NK_WINDOW_SCALABLE))
        {

            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;
            nk_layout_row_static(ctx, 30 * scaleY, 80 * scaleX, 1);
            if (nk_button_label(ctx, "Load Model"))
            {
                OPENFILENAMEA f = {sizeof(OPENFILENAMEA)};
                f.lpstrFilter = "glb files\0*.glb\0obj files\0*.obj\0gltf files\0*.gltf\0";
                f.lpstrTitle = "Load Model";
                static char buff[MAX_PATH] = {};
                f.nMaxFile = sizeof(buff);
                f.lpstrFile = buff;
                GetOpenFileNameA(&f);
                printf("%s",f.lpstrFile);
                freeModel(model);
                model->path = f.lpstrFile;
                loadModel(model);
                printf("numMeshes: %u\n",model->numMeshes);
                if (model->numMeshes > 0)
                {
                    printf("mesh[0] numVertices %u, numIndices %u\n",model->meshes[0].numVertices,model->meshes[0].numIndices);
                }
            }

            nk_layout_row_dynamic(ctx, 20 *scaleY, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25 * scaleY, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(*bg), nk_vec2(nk_widget_width(ctx),400 * scaleY))) {
                nk_layout_row_dynamic(ctx, 120 * scaleY, 1);
                *bg = nk_color_picker(ctx, *bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25 * scaleY, 1);
                bg->r = nk_propertyf(ctx, "#R:", 0, bg->r, 1.0f, 0.01f,0.005f);
                bg->g = nk_propertyf(ctx, "#G:", 0, bg->g, 1.0f, 0.01f,0.005f);
                bg->b = nk_propertyf(ctx, "#B:", 0, bg->b, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }

            nk_layout_row_dynamic(ctx, 25 * scaleY, 1);
            nk_label(ctx, "Rotation:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25 * scaleY, 3);
            static transformation t  = { .rotation = {0.0f, 0.0f, 0.0f}, .scale = {1.0f, 1.0f, 1.0f}, .position = {0.0f, 0.0f, 0.0f} };
            nk_property_float(ctx, "#X", -180, &t.rotation[0], 180, 1.0f, 0.2f * scaleX);
            nk_property_float(ctx, "#Y", -180, &t.rotation[1], 180, 1.0f, 0.2f * scaleX);
            nk_property_float(ctx, "#Z", -180, &t.rotation[2], 180, 1.0f, 0.2f * scaleX);
            nk_label(ctx, "Scale:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25 * scaleY, 3);
            nk_property_float(ctx, "#X", -100, &t.scale[0], 100, 1.0f, 0.2f * scaleX);
            nk_property_float(ctx, "#Y", -100, &t.scale[1], 100, 1.0f, 0.2f * scaleX);
            nk_property_float(ctx, "#Z", -100, &t.scale[2], 100, 1.0f, 0.2f * scaleX);

            nk_label(ctx, "Position:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25 * scaleY, 3);
            nk_property_float(ctx, "#X", -1000000000000000000000000.0f, &t.position[0], 1000000000000000000000000.0f, 1.0f, 0.01f * scaleX);
            nk_property_float(ctx, "#Y", -1000000000000000000000000.0f, &t.position[1], 1000000000000000000000000.0f, 1.0f, 0.01f * scaleX);
            nk_property_float(ctx, "#Z", -1000000000000000000000000.0f, &t.position[2], 1000000000000000000000000.0f, 1.0f, 0.01f * scaleX);


           transformModel(shader,t);

        }
        nk_end(ctx);
    }
}

    void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    #define MAX_VERTEX_BUFFER 512 * 1024
    #define MAX_ELEMENT_BUFFER 128 * 1024

    int main() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
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
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        initCamera(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);
        Shader triShader;
        buildShader(&triShader,"vs.vs","fs.fs");


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);






        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrChannels;
        mat4 view;
        mat4 projection;
        Model model = {0};


        struct nk_glfw glfw = {0};
        struct nk_context *ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);


    struct nk_colorf bg = {0.65f,0.65f,0.65f,1.0f};
    resizeFont(&glfw,ctx,1);
        while (!glfwWindowShouldClose(window))
        {

            nk_glfw3_new_frame(&glfw);
            cameraInput(window);
            //nuklearFrame(ctx,&bg,window,&glfw,&model,&triShader);
            glClearColor(1.0f,0.0f,0.0f,1.0f);
            glClearColor(bg.r, bg.g, bg.b, 1.0f - bg.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            //glBindTexture(GL_TEXTURE_2D, texture);
            useShader(&triShader);
            nuklearFrame(ctx,&bg,window,&glfw,&model,&triShader);

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
            drawModel(&model, &triShader);
            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR)
                printf("error %x\n", err);


            nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwTerminate();
        return 0;
    }

