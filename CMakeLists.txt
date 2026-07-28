cmake_minimum_required(VERSION 3.20)
project(renderer C)

set(CMAKE_C_STANDARD 11)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(cglm CONFIG REQUIRED)

add_executable(renderer main.c
        shader.h
        shader.c
        stb_image.h)

target_link_libraries(renderer PRIVATE glfw glad::glad cglm::cglm)