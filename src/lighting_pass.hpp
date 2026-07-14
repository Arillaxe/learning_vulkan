#ifndef LIGHTING_PASS_HPP
#define LIGHTING_PASS_HPP

#include <vulkan/vulkan_raii.hpp>
#include <render_pass.hpp>
#include <geomerty_pass.hpp>
#include <light.hpp>

class LightingPass : public RenderPass
{
private:
  GeometryPass *geometryPass;
  std::vector<Light *> lights;

public:
  LightingPass(const std::string &name, GeometryPass *gPass)
      : RenderPass(name), geometryPass(gPass)
  {
    // Add dependency on geometry pass
    AddDependency(gPass->GetName());
  }

  void AddLight(Light *light)
  {
    lights.push_back(light);
  }

  void RemoveLight(Light *light)
  {
    auto it = std::find(lights.begin(), lights.end(), light);
    if (it != lights.end())
    {
      lights.erase(it);
    }
  }

protected:
  void BeginPass(vk::raii::CommandBuffer &commandBuffer) override
  {
    // Begin rendering with dynamic rendering
    vk::RenderingInfoKHR renderingInfo;

    // Set up color attachment for the lighting pass
    vk::RenderingAttachmentInfoKHR colorAttachment;
    colorAttachment.setImageView(GetRenderTarget()->GetColorImageView())
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));

    // Configure rendering info
    renderingInfo.setRenderArea(vk::Rect2D({0, 0}, {GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight()}))
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachment);

    // Begin dynamic rendering
    commandBuffer.beginRendering(renderingInfo);
  }

  void Render(vk::raii::CommandBuffer &commandBuffer) override
  {
    // Bind G-buffer textures from the geometry pass
    auto gBuffer = geometryPass->GetRenderTarget();

    // Set up descriptor sets for G-buffer textures
    // With dynamic rendering, we access the G-buffer textures directly as shader resources
    // rather than as subpass inputs

    // Render full-screen quad with lighting shader
    // ...

    // For each light
    for (auto light : lights)
    {
      // Set light properties
      // ...

      // Draw light volume
      // ...
    }
  }

  void EndPass(vk::raii::CommandBuffer &commandBuffer) override
  {
    // End dynamic rendering
    commandBuffer.endRendering();
  }
};

#endif // LIGHTING_PASS_HPP
