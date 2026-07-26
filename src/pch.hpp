#ifndef PCH_HPP
#define PCH_HPP

// Precompiled header: only stable, expensive-to-parse headers belong here.
// These change rarely, so compiling them once and reusing the .gch removes the
// bulk of the per-translation-unit parse cost (dominated by vulkan_raii.hpp,
// which every TU pulls in). This is force-included into every TU by the build.

// Heavy standard library headers used across the project.
#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

// GLM. The force macro must be defined before any glm header is included so the
// precompiled configuration matches how the rest of the project uses glm.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Vulkan. vk.hpp sets the required VULKAN_HPP_* macros before pulling in
// vulkan_raii.hpp (the single heaviest header in the build).
#include <renderer/vk.hpp>

// GLFW must be included with Vulkan support and after the Vulkan headers,
// mirroring the ordering in core/window.hpp.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <perlin_noise.hpp>

#endif // PCH_HPP
