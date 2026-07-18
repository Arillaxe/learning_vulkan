#ifndef SIMPLE_PASS_HPP
#define SIMPLE_PASS_HPP

#include <render_pass.hpp>
#include <render_target.hpp>
#include <mesh_component.hpp>
#include <transform_component.hpp>
#include <culling_system.hpp>
#include <shader_resource.hpp>
#include <vertex.hpp>
#include <vk_init.hpp>
#include <swapchain.hpp>

class SimplePass : public RenderPass
{
private:
    CullingSystem *cullingSystem;
    RenderTarget target;
    Shader *vertexShader;
    Shader *fragmentShader;
    VkInit &vkInit;
    Swapchain &swapchain;

public:
    SimplePass(VkInit &init, const std::string &name, CullingSystem *culling, Shader *vertShader, Shader *fragShader, Swapchain &swap)
        : RenderPass(name), target(1280, 720), cullingSystem(culling),
          vertexShader(vertShader), fragmentShader(fragShader), vkInit(init), swapchain(swap)
    {
        SetRenderTarget(&target);
    }

protected:
    void CreateDescriptorSetLayout()
    {
        std::array<vk::DescriptorSetLayoutBinding, 4> bindings{{
            {
                .binding = 0,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
            },
            {
                .binding = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
            },
        }};

        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };

        descriptorSetLayout = vk::raii::DescriptorSetLayout(vkInit.getDevice(), layoutInfo);
    }

    void CreateDescriptorSets()
    {
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = descriptorSetLayout,
        };

        descriptorSets = vkInit.getDevice().allocateDescriptorSets(allocInfo);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DescriptorBufferInfo bufferInfo{
                .buffer = uniformBuffers[i],
                .offset = 0,
                .range = sizeof(UniformBufferObject),
            };
            vk::DescriptorImageInfo imageInfo{
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };
            vk::DescriptorBufferInfo computeBufferInfoLastFrame{
                .buffer = computeStorageBuffers[(i + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT],
                .offset = 0,
                .range = sizeof(Particle) * PARTICLE_COUNT,
            };
            vk::DescriptorBufferInfo computeBufferInfoCurrentFrame{
                .buffer = computeStorageBuffers[i],
                .offset = 0,
                .range = sizeof(Particle) * PARTICLE_COUNT,
            };
            std::array<vk::WriteDescriptorSet, 4> descriptorWrite{{
                {
                    .dstSet = descriptorSets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &bufferInfo,
                },
                {
                    .dstSet = descriptorSets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &imageInfo,
                },
                {
                    .dstSet = descriptorSets[i],
                    .dstBinding = 2,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &computeBufferInfoLastFrame,
                },
                {
                    .dstSet = descriptorSets[i],
                    .dstBinding = 3,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &computeBufferInfoCurrentFrame,
                },
            }};

            device.updateDescriptorSets(descriptorWrite, {});
        }
    }

    void CreatePipeline() override
    {
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertexShader->getShaderModule(),
            .pName = "vertMain",
        };

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragmentShader->getShaderModule(),
            .pName = "fragMain",
        };

        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescription();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
            .topology = vk::PrimitiveTopology::eTriangleList,
        };

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::ePrimitiveTopology};
        vk::PipelineDynamicStateCreateInfo dynamicState{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data(),
        };

        vk::PipelineViewportStateCreateInfo viewportState{
            .viewportCount = 1,
            .scissorCount = 1,
        };

        vk::PipelineRasterizationStateCreateInfo rasterizer{
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False,
            .lineWidth = 1.0f,
        };

        vk::PipelineMultisampleStateCreateInfo multisampling{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::True,
            .minSampleShading = 0.2f,
        };

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = vk::False,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
        };

        vk::PushConstantRange pushRange{
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(uint32_t),
        };

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &*descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushRange,
        };

        pipelineLayout = vk::raii::PipelineLayout(vkInit.getDevice(), pipelineLayoutInfo);

        vk::PipelineDepthStencilStateCreateInfo depthStencil{
            .depthTestEnable = vk::True,
            .depthWriteEnable = vk::True,
            .depthCompareOp = vk::CompareOp::eLess,
            .depthBoundsTestEnable = vk::False,
            .stencilTestEnable = vk::False,
        };

        vk::StructureChain<
            vk::GraphicsPipelineCreateInfo,
            vk::PipelineRenderingCreateInfo>
            pipelineCreateInfoChain = {
                {
                    .stageCount = 2,
                    .pStages = shaderStages,
                    .pVertexInputState = &vertexInputInfo,
                    .pInputAssemblyState = &inputAssembly,
                    .pViewportState = &viewportState,
                    .pRasterizationState = &rasterizer,
                    .pMultisampleState = &multisampling,
                    .pDepthStencilState = &depthStencil,
                    .pColorBlendState = &colorBlending,
                    .pDynamicState = &dynamicState,
                    .layout = pipelineLayout,
                    .renderPass = nullptr,
                },
                {.colorAttachmentCount = 1,
                 .pColorAttachmentFormats = &swapchain.getSwapChainSurfaceFormat().format,
                 .depthAttachmentFormat = swapchain.findDepthFormat()},
            };

        pipeline = vk::raii::Pipeline(vkInit.getDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
    }

    void BeginPass(vk::raii::CommandBuffer &commandBuffer) override
    {
        // Begin rendering with dynamic rendering
        vk::RenderingInfoKHR renderingInfo;

        // Set up color attachment
        vk::RenderingAttachmentInfoKHR colorAttachment;
        colorAttachment.setImageView(GetRenderTarget()->GetColorImageView())
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));

        // Set up depth attachment
        vk::RenderingAttachmentInfoKHR depthAttachment;
        depthAttachment.setImageView(GetRenderTarget()->GetDepthImageView())
            .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearDepthStencilValue(1.0f, 0));

        // Configure rendering info
        renderingInfo.setRenderArea(vk::Rect2D({0, 0}, {target.GetWidth(), target.GetHeight()}))
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

                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
                commandBuffer.bindVertexBuffers(0, *meshComponent->getMesh()->getVertextBuffer(), {0});
                commandBuffer.bindIndexBuffer(*meshComponent->getMesh()->getIndexBuffer(), 0, vk::IndexType::eUint32);
                commandBuffer.bindDescriptorSets()
            }
        }
    }

    void EndPass(vk::raii::CommandBuffer &commandBuffer) override
    {
        // End dynamic rendering
        commandBuffer.endRendering();
    }
};

#endif // SIMPLE_PASS_HPP
