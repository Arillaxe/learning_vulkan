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
#include <renderer/gpu_chunk_mesh.hpp>
#include <core/thread_queue.hpp>
#include <core/chunk_mesh.hpp>
#include <unordered_map>
#include <renderer/gui.hpp>

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
  std::unordered_map<ChunkPos, GPUChunkMesh, ChunkPosHash> chunkMeshes;
  ThreadQueue<GPUChunkMesh> &loadQueue;
  ThreadQueue<ChunkPos> &unloadQueue;
  GUI gui;

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
  Renderer(Window &win, Scene &_scene, Camera &cam, ThreadQueue<GPUChunkMesh> &lQueue, ThreadQueue<ChunkPos> &uQueue);

  VkResource &getVkResource() { return vkResource; }

  void render();
  void waitIdle();
  void drawGUI();
};

#endif // RENDERER_HPP
