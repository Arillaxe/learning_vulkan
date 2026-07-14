#ifndef GEOMERTY_PASS_HPP
#define GEOMERTY_PASS_HPP

#include <render_pass.hpp>
#include <render_target.hpp>
#include <mesh_component.hpp>
#include <transform_component.hpp>
#include <culling_system.hpp>

class GeometryPass : public RenderPass
{
private:
  CullingSystem *cullingSystem;

  // G-buffer textures
  RenderTarget *gBuffer;

public:
  GeometryPass(const std::string &name, CullingSystem *culling)
      : RenderPass(name), cullingSystem(culling)
  {
    // Create G-buffer render target
    gBuffer = new RenderTarget(1920, 1080); // Example resolution
    SetRenderTarget(gBuffer);
  }

  ~GeometryPass() override
  {
    delete gBuffer;
  }

protected:
  void BeginPass(vk::raii::CommandBuffer &commandBuffer) override
  {
    // Begin rendering with dynamic rendering
    vk::RenderingInfoKHR renderingInfo;

    // Set up color attachment
    vk::RenderingAttachmentInfoKHR colorAttachment;
    colorAttachment.setImageView(gBuffer->GetColorImageView())
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));

    // Set up depth attachment
    vk::RenderingAttachmentInfoKHR depthAttachment;
    depthAttachment.setImageView(gBuffer->GetDepthImageView())
        .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearDepthStencilValue(1.0f, 0));

    // Configure rendering info
    renderingInfo.setRenderArea(vk::Rect2D({0, 0}, {gBuffer->GetWidth(), gBuffer->GetHeight()}))
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachment)
        .setPDepthAttachment(&depthAttachment);

    // Begin dynamic rendering
    commandBuffer.beginRendering(renderingInfo);
  }

  void Render(vk::raii::CommandBuffer &commandBuffer) override
  {
    // Get visible entities
    const auto &visibleEntities = cullingSystem->GetVisibleEntities();

    // Render each entity to G-buffer
    for (auto entity : visibleEntities)
    {
      auto meshComponent = entity->getComponent<MeshComponent>();
      auto transformComponent = entity->getComponent<TransformComponent>();

      if (meshComponent && transformComponent)
      {
        // Bind pipeline for G-buffer rendering
        // ...

        // Set model matrix
        // ...

        // Draw mesh
        // ...
      }
    }
  }

  void EndPass(vk::raii::CommandBuffer &commandBuffer) override
  {
    // End dynamic rendering
    commandBuffer.endRendering();
  }
};

#endif // GEOMERTY_PASS_HPP
