#!/usr/bin/env bash

set -e

GLFW_PATH="$(brew --prefix glfw)"

includes="-Isrc -I${VULKAN_SDK}/include -I${GLFW_PATH}/include"

links="
-L${GLFW_PATH}/lib
-L${VULKAN_SDK}/lib
-lglfw
-lvulkan
-Wl,-rpath,${VULKAN_SDK}/lib
-framework Cocoa
-framework IOKit
-framework CoreVideo
-framework QuartzCore
-framework Metal
"

defines="-DDEBUG"

echo "Building main..."

g++ -std=c++20 \
    $includes \
    $defines \
    src/main.cpp \
    $links \
    -o main

echo "Build complete."
