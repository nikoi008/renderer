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

    void nuklearFrame(struct nk_context* ctx,struct nk_colorf bg)
    {
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;
            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                fprintf(stdout, "button pressed\n");

            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);
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
        backpack.path = "bunny.obj";
        loadModel(&backpack);
        printf("numMeshes: %u\n", backpack.numMeshes);
        if (backpack.numMeshes > 0)
        {
            printf("mesh[0] numVertices %u, numIndices %u\n",backpack.meshes[0].numVertices, backpack.meshes[0].numIndices);
        }

        struct nk_glfw glfw = {0};
        struct nk_context *ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);

        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        nk_glfw3_font_stash_end(&glfw);

        struct nk_colorf bg;

        while (!glfwWindowShouldClose(window))
        {

            nk_glfw3_new_frame(&glfw);
            cameraInput(window);
            nuklearFrame(ctx,bg);
            glClearColor(bg.r, bg.g, bg.b, bg.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
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

