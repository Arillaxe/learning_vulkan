@echo off

echo "Compiling shaders..."

%VULKAN_SDK%/bin/slangc.exe ./shaders/shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -entry compMain -o slang.spv
