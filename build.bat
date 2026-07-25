@echo off

set includes=-Isrc -I%VULKAN_SDK%/Include -I%GLFW_PATH%/include -I./include
set links=-L%GLFW_PATH%/lib-mingw-w64 -lglfw3 -L%VULKAN_SDK%/Lib -lvulkan-1 -lgdi32
set defines=-D DEBUG

echo Building main...

for /f %%A in ('powershell -NoProfile -Command "[DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()"') do set "START=%%A"

g++ -std=c++20 %includes% %defines% src/main_new.cpp %links% -o main.exe

for /f %%A in ('powershell -NoProfile -Command "[math]::Round(([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds() - %START%) / 1000, 2)"') do set "ELAPSED=%%A"

echo Build finished in %ELAPSED% seconds
