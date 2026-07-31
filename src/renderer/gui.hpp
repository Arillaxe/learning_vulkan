#ifndef GUI_HPP
#define GUI_HPP

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <core/window.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vk/vk_swapchain.hpp>

static void check_vk_result(VkResult err)
{
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

class GUI
{
private:
  Window &window;
  VkContext &vkContext;
  VkResource &vkResource;
  VkSwapchain &vkSwapchain;

public:
  GUI(Window &win, VkContext &context, VkResource &resource, VkSwapchain &swapchain)
      : window(win), vkContext(context), vkResource(resource), vkSwapchain(swapchain)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    // io.ConfigNavMoveSetMousePos = true;
    io.DisplaySize.x = (float)swapchain.getExtent().width;
    io.DisplaySize.y = (float)swapchain.getExtent().height;

    ImGui::GetStyle().FontScaleMain = 1.5f;

    ImGui_ImplGlfw_InitForVulkan(window.getGLFWWindowHandle(), true);

    vk::Format colorFormat = swapchain.getSurfaceFormat().format;
    vk::Format depthFormat = swapchain.findDepthFormat();

    vk::PipelineRenderingCreateInfo renderingInfo{
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
        .depthAttachmentFormat = depthFormat,
    };

    ImGui_ImplVulkan_PipelineInfo pipelineInfo;
    pipelineInfo.PipelineRenderingCreateInfo = renderingInfo;
    pipelineInfo.RenderPass = nullptr;
    pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_4_BIT;

    ImGui_ImplVulkan_InitInfo init_info{
        .Instance = *vkContext.getInstance(),
        .PhysicalDevice = *vkContext.getPhysicalDevice(),
        .Device = *vkContext.getDevice(),
        .QueueFamily = vkContext.getQueueFamilyIndex(),
        .Queue = *vkContext.getQueue(),
        .DescriptorPool = *vkResource.getDescriptorPool(),
        .MinImageCount = 2,
        .ImageCount = 2,
        .UseDynamicRendering = true,
        .CheckVkResultFn = check_vk_result,
    };

    init_info.PipelineInfoMain = pipelineInfo;

    ImGui_ImplVulkan_Init(&init_info);
  }

  ~GUI()
  {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void beginFrame()
  {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void endFrame(vk::raii::CommandBuffer &commandBuffer)
  {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);
  }
};

#endif // GUI_HPP
