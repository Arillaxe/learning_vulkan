#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <core/window.hpp>
#include <renderer/vk/vk_context.hpp>
#include <renderer/vk/vk_command.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vk/vk_synchronization.hpp>
#include <renderer/vk/vk_swapchain.hpp>
#include <renderer/pipelines/main_pipeline.hpp>
#include <core/scene.hpp>
#include <core/camera.hpp>

class Renderer
{
private:
  Window &window;
  VkContext vkContext;
  VkCommand vkCommand;
  VkResource vkResource;
  VkSynchronization vkSynchronization;
  VkSwapchain vkSwapchain;
  Scene &scene;
  vk::raii::CommandBuffers commandBuffers;
  MainPipeline mainPipeline;
  Camera &camera;
  vk::raii::QueryPool queryPool;

  void transition_image_layout(
      vk::raii::CommandBuffer &commandBuffer,
      vk::Image image,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      vk::AccessFlags2 src_access_mask,
      vk::AccessFlags2 dst_access_mask,
      vk::PipelineStageFlags2 src_stage_mask,
      vk::PipelineStageFlags2 dst_stage_mask,
      vk::ImageAspectFlags image_aspect_flags);

public:
  Renderer(Window &win, Scene &_scene, Camera &cam);

  VkResource &getVkResource() { return vkResource; }

  void render();
  void waitIdle();
};

#endif // RENDERER_HPP
