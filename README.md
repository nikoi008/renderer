# C OpenGl renderer
[](images/banner.png) 
# What is this?
This project is a renderer designed to load 3d model files!  
It currently has best support for `.glb` files, but support for `.obj` and `gltf` files are available though require extra setup
# Controls
You use WASD to move the camera forwards/backwards/left/right, but right click and moving the mouse around lets you rotate the camera

There is a dashboard on the left of the window, with controls here  

| Element           | Control                                                                                                                         |  
|-------------------|---------------------------------------------------------------------------------------------------------------------------------|
| Load model        | Button: press to open a prompt to load the model of choice                                                                      |
| Background colour | Press to open/close a window which lets you change the background colour                                                        |
|Change bg colour window | Colour picker that is selected with the mouse, and sliders that interact when you click and slide left or right with them       |
| Rotate/Scale/Position sliders | Interact with the sliders by clicking on them and slide them left/right. Each slider corresponds to the specific X/Y/Z property |

# Compatibility 
Currently, the model loader is in a very early stage, so here is a small compilation of stuff that I found works and doesn't work.  
I have included 3d models in the release that contain 3 of the compatibility levels
## Best compatibility
Models that work the best are .glb files with 1 UV map and 1 texcoord, along with them being a native .glb file (not converted from an fbx or other file format)

## Moderate compatibility

.glb files with pbr materials will load everything but the pbr stuff, causing the non loaded stuff to be either a neon colour or greyish black  
SOME files that were converted to a .glb file (may have stuff like a huge scale or not properly mapped textures)  
.obj files require a specific setup(https://www.youtube.com/watch?v=4DQquG_o-Ac)  
.gltf are relatively untested but have the same incompatibilities as .glb files

## Worst compatibility
proprietary model formats such as .fbx  
MOST files that were converted to a .glb
Files with more than one uv map and more than one texcoord

# Building
You must first have the following tools to build it  
- [CMake](https://cmake.org/download/) 3.20+
- [Ninja](https://github.com/ninja-build/ninja/releases)
- MinGW-w64
- [vcpkg](https://github.com/microsoft/vcpkg)

Firstly, you must install all the libraries needed:  
```vcpkg install glfw3:x64-mingw-dynamic glad:x64-mingw-dynamic cglm:x64-mingw-dynamic assimp:x64-mingw-dynamic```  

Then configure cmake and build
```
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic \
  -B cmake-build-debug
```
```
cmake --build cmake-build-debug --target renderer -j 8
```
build files will then be produced or you can run using ```./cmake-build-debug/renderer.exe```
# License
This project is licensed under the MIT license