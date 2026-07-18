@echo off

set includes=-Isrc -I%VULKAN_SDK%/Include -I%GLFW_PATH%/include -I./include
set links=-L%GLFW_PATH%/lib-mingw-w64 -lglfw3 -L%VULKAN_SDK%/Lib -lvulkan-1 -lgdi32
set defines=-D DEBUG

echo "Building main..."

g++ -std=c++20 %includes% %defines% src/main_new.cpp %links% -o main.exe

echo "Build finished!"
