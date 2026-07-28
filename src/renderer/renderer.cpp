#include <renderer/renderer.hpp>

#include <ecs/components/mesh_component.hpp>
#include <ecs/components/transform_component.hpp>
#include <renderer/push.hpp>
#include <cassert>
#include <core/chunk.hpp>
#include <iostream>
#include <algorithm>
#include "renderer.hpp"

Renderer::Renderer(Window &win, Scene &_scene, Camera &cam, ThreadQueue<GPUChunkMesh> &lQueue, ThreadQueue<ChunkPos> &uQueue)
		: window(win),
			vkContext(win),
			vkCommand(vkContext),
			vkResource(vkContext, vkCommand),
			vkSynchronization(vkContext),
			vkSwapchain(vkContext, win, vkResource, vkSynchronization),
			scene(_scene),
			commandBuffers(vkCommand.createCommandBuffers(1)),
			mainPipeline(vkContext, vkSwapchain, vkResource),
			camera(cam),
			queryPool(
					vkContext.getDevice(),
					vk::QueryPoolCreateInfo{}
							.setQueryType(vk::QueryType::eTimestamp)
							.setQueryCount(2)),
			loadQueue(lQueue),
			unloadQueue(uQueue),
			gui(window, vkContext, vkResource, vkSwapchain) {}

void Renderer::transition_image_layout(
		vk::raii::CommandBuffer &commandBuffer,
		vk::Image image,
		vk::ImageLayout old_layout,
		vk::ImageLayout new_layout,
		vk::AccessFlags2 src_access_mask,
		vk::AccessFlags2 dst_access_mask,
		vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask,
		vk::ImageAspectFlags image_aspect_flags)
{
	vk::ImageMemoryBarrier2 barrier = {
			.srcStageMask = src_stage_mask,
			.srcAccessMask = src_access_mask,
			.dstStageMask = dst_stage_mask,
			.dstAccessMask = dst_access_mask,
			.oldLayout = old_layout,
			.newLayout = new_layout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
					.aspectMask = image_aspect_flags,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
			},
	};

	vk::DependencyInfo dependency_info = {
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
	};

	commandBuffer.pipelineBarrier2(dependency_info);
}

void Renderer::render()
{
	auto start = std::chrono::high_resolution_clock::now();

	vkSynchronization.waitDrawFence();
	std::array<uint64_t, 2> timestamps{};

	auto queryPoolResult = (*vkContext.getDevice()).getQueryPoolResults(*queryPool,
																																			0,									// first query
																																			2,									// query count
																																			sizeof(timestamps), // data size
																																			timestamps.data(),	// data
																																			sizeof(uint64_t),		// stride
																																			vk::QueryResultFlagBits::e64);

	double gpuMs =
			(timestamps[1] - timestamps[0]) *
			vkContext.getPhysicalDevice().getProperties().limits.timestampPeriod /
			1'000'000.0;

	//   std::cout << gpuMs << std::endl;

	uint32_t imageIndex = vkSwapchain.acquireNextImage();
	auto &commandBuffer = commandBuffers[0];

	commandBuffer.reset();

	auto &entities = scene.getEntities();

	commandBuffer.begin({});

	commandBuffer.resetQueryPool(*queryPool, 0, 2);
	commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe, *queryPool, 0);

	transition_image_layout(
			commandBuffer,
			vkSwapchain.getImages()[imageIndex],
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);

	transition_image_layout(
			commandBuffer,
			*vkSwapchain.getColorImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);

	transition_image_layout(
			commandBuffer,
			*vkSwapchain.getDepthImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo attachmentInfo = {
			.imageView = vkSwapchain.getColorImageView(),
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.resolveMode = vk::ResolveModeFlagBits::eAverage,
			.resolveImageView = vkSwapchain.getImageViews()[imageIndex],
			.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = clearColor,
	};

	vk::RenderingAttachmentInfo depthAttachmentInfo = {
			.imageView = vkSwapchain.getDepthImageView(),
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eDontCare,
			.clearValue = clearDepth,
	};

	vk::RenderingInfo renderingInfo = {
			.renderArea = {.offset = {0, 0}, .extent = vkSwapchain.getExtent()},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo,
	};

	commandBuffer.beginRendering(renderingInfo);

	vk::Viewport viewport{
			0.0f,
			0.0f,
			static_cast<float>(vkSwapchain.getExtent().width),
			static_cast<float>(vkSwapchain.getExtent().height),
			0.0f,
			1.0f,
	};
	vk::Rect2D scissor{vk::Offset2D{0, 0}, vkSwapchain.getExtent()};

	commandBuffer.setViewport(0, viewport);
	commandBuffer.setScissor(0, scissor);
	commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mainPipeline.getPipeline());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mainPipeline.getPipelineLayout(), 0, *mainPipeline.getDescriptorSet(), nullptr);

	for (auto &entity : entities)
	{
		if (!entity.isActive())
			continue;

		auto meshComponent = entity.getComponent<MeshComponent>();
		auto transformComponent = entity.getComponent<TransformComponent>();

		if (!meshComponent || !transformComponent)
			continue;

		auto &mesh = meshComponent->getMesh();
		auto &indices = mesh.getIndices();

		commandBuffer.bindVertexBuffers(0, *mesh.getVertexBuffer(), {0});
		commandBuffer.bindIndexBuffer(*mesh.getIndexBuffer(), 0, vk::IndexTypeValue<uint32_t>::value);
		PushConstants pushConstants;

		pushConstants.model = transformComponent->getTransformMatrix();

		commandBuffer.pushConstants<PushConstants>(mainPipeline.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, pushConstants);
		commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}

	PushConstants pushConstants;
	pushConstants.model = glm::mat4(1.0f);

	unloadQueueSize = unloadQueue.getSize();

	while (unloadQueue.tryPop(toUnload))
	{
		chunkMeshes.erase(toUnload);
	}

	loadQueueSize = loadQueue.getSize();

	while (loadQueue.tryPop(toLoad))
	{
		chunkMeshes.insert_or_assign(toLoad.getChunkPos(), std::move(toLoad));

		// auto it = chunkMeshes.find(toLoad.chunkPos);
		// if (it == chunkMeshes.end())
		// 	it = chunkMeshes.try_emplace(toLoad.chunkPos, &vkResource, toLoad.chunkPos).first;
		// it->second.generateRenderMesh(toLoad.vertices, toLoad.indices);
	}

	totalIndices = 0;

	for (auto &[pos, chunkMesh] : chunkMeshes)
	{
		commandBuffer.bindVertexBuffers(0, *chunkMesh.getVertexBuffer(), {0});
		commandBuffer.bindIndexBuffer(*chunkMesh.getIndexBuffer(), 0, vk::IndexTypeValue<uint32_t>::value);
		commandBuffer.pushConstants<PushConstants>(mainPipeline.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, pushConstants);

		int indicesCount = chunkMesh.getIndicesCount();

		totalIndices += indicesCount;

		commandBuffer.drawIndexed(indicesCount, 1, 0, 0, 0);
	}

	// GUI

	gui.beginFrame();

	drawGUI();

	gui.endFrame(commandBuffer);

	// end GUI

	commandBuffer.endRendering();

	transition_image_layout(
			commandBuffer,
			vkSwapchain.getImages()[imageIndex],
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			{},
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eBottomOfPipe,
			vk::ImageAspectFlagBits::eColor);

	commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, *queryPool, 1);

	commandBuffer.end();

	// vkContext.getDevice().waitIdle();

	mainPipeline.getVkUbo().updateUniformBuffer(camera);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const vk::SubmitInfo submitInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*vkSynchronization.getPresentCompleteSemaphore(),
			.pWaitDstStageMask = &waitDestinationStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &*commandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*vkSynchronization.getRenderFinishedSemaphore(),
	};

	vkContext.getQueue().submit(submitInfo, *vkSynchronization.getDrawFence());

	const vk::PresentInfoKHR presentInfoKHR{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*vkSynchronization.getRenderFinishedSemaphore(),
			.swapchainCount = 1,
			.pSwapchains = &*vkSwapchain.getSwapchain(),
			.pImageIndices = &imageIndex,
	};

	auto result = vkContext.getQueue().presentKHR(presentInfoKHR);

	if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || window.framebufferResized)
	{
		window.framebufferResized = false;

		vkSwapchain.recreateSwapchain();
	}
	else
	{
		assert(result == vk::Result::eSuccess);
	}

	auto end = std::chrono::high_resolution_clock::now();

	double cpuMs =
			std::chrono::duration<double, std::milli>(end - start).count();

	//   std::cout << cpuMs << std::endl;
}

void Renderer::waitIdle()
{
	vkContext.getDevice().waitIdle();
}

void Renderer::drawGUI()
{
	ImGui::Text("Chunks loaded: %d", chunkMeshes.size());
	ImGui::Text("Load queue size: %d", loadQueueSize);
	ImGui::Text("Unload queue size: %d", unloadQueueSize);
	ImGui::Text("Triangles: %d", totalIndices / 3);
	ImGui::Text("Destroyed meshes: %d", GPUChunkMesh::destroyed);
}
