#ifndef MAIN_PIPELINE_HPP
#define MAIN_PIPELINE_HPP

#include <renderer/vk/vk_pipeline.hpp>
#include <renderer/vertex.hpp>
#include <renderer/ubo.hpp>

class MainPipeline : public Vk_Pipeline
{
protected:
  vk::raii::PipelineLayout createPipelineLayout() override
  {
    vk::PushConstantRange pushRange{
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(PushConstants),
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushRange,
    };

    return vk::raii::PipelineLayout(vkContext.getDevice(), pipelineLayoutInfo);
  }

  vk::raii::Pipeline createPipeline() override
  {
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shader.getShaderModule(),
        .pName = "vertMain",
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shader.getShaderModule(),
        .pName = "fragMain",
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = getBindingDescription();
    auto attributeDescriptions = getAttributeDescription();

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
        .rasterizationSamples = vk::SampleCountFlagBits::e4,
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
             .pColorAttachmentFormats = &vkSwapchain.getSurfaceFormat().format,
             .depthAttachmentFormat = findDepthFormat()},
        };

    return vk::raii::Pipeline(vkContext.getDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
  }

  vk::raii::DescriptorSetLayout createDescriptorSetLayout() override
  {
    std::array<vk::DescriptorSetLayoutBinding, 1> bindings{{
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
        },
    }};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    return vk::raii::DescriptorSetLayout(vkContext.getDevice(), layoutInfo);
  }

  vk::raii::DescriptorSet createDescriptorSet() override
  {
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = vkResource.getDescriptorPool(),
        .descriptorSetCount = 1,
        .pSetLayouts = &*descriptorSetLayout,
    };

    auto descriptorSet = std::move(vkContext.getDevice().allocateDescriptorSets(allocInfo).front());

    vk::DescriptorBufferInfo bufferInfo{
        .buffer = vkUbo.getUniformBuffer(),
        .offset = 0,
        .range = sizeof(UniformBufferObject),
    };

    std::array<vk::WriteDescriptorSet, 1> descriptorWrite{{
        {
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo,
        },
    }};

    vkContext.getDevice().updateDescriptorSets(descriptorWrite, {});

    return descriptorSet;
  }

public:
  MainPipeline(VkContext &context, VkSwapchain &swapchain, VkResource &resource)
      : Vk_Pipeline(context, swapchain, resource, "slang.spv")
  {
    initialize();
  }
};

#endif // MAIN_PIPELINE_HPP
