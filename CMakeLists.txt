cmake_minimum_required(VERSION 3.20)
project(renderer C)

set(CMAKE_C_STANDARD 11)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(renderer main.c
        shader.h
        shader.c)

target_link_libraries(renderer PRIVATE glfw glad::glad)