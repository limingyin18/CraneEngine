#include "Render.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void Render::assemblePrimitives()
{
	stack<shared_ptr<Actor>> stackActor;
	for(auto actor : scene)
	{
		actor->transformer.setTransformParent(&transformWorld);
		actor->updateTransform();
		stackActor.push(actor);
	}

	while(!stackActor.empty())
	{
		shared_ptr<Actor> actor = stackActor.top();
		stackActor.pop();

		for(auto gp : actor->primitives)
			renderables.push_back(gp);

		for(auto child : actor->childs)
			stackActor.push(child);
	}
}

void Render::buildRenderable()
{
	LOGI("��������Ⱦ����");

	compactDraws();

	modelMatrixOffset = padUniformBufferSize(sizeof(Eigen::Matrix4f), physicalDevice.getProperties());
	const size_t modelMatrixBufferSize = renderables.size() * modelMatrixOffset;
	modelMatrix.resize(modelMatrixBufferSize);
	auto modelMatrixPtr = modelMatrix.data();

	uint32_t offset = 0;
	for (auto& d : draws)
	{
		auto& v = renderables[d.first];
		v->vertBufferOffset = offset;
		for (size_t i = 0; i < v->mesh->data.size(); i++)
			vertices.push_back(v->mesh->data[i]);
		for (size_t i = 0; i < v->mesh->indices.size(); i++)
			indices.push_back(offset + v->mesh->indices[i]);
		offset += v->mesh->data.size();

		for (uint32_t i = 0; i < d.count; ++i)
		{
			*(reinterpret_cast<Eigen::Matrix4f*>(modelMatrixPtr)) = renderables[d.first + i]->transformer.getTransformWorld();
			modelMatrixPtr = modelMatrixPtr + modelMatrixOffset;
		}
	}

	modelMatrixBuffer.create(*vmaAllocator, modelMatrixBufferSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	modelMatrixBuffer.update(modelMatrix.data());

	modelMatrixBufferDescriptorInfo.buffer = modelMatrixBuffer.buffer;
	modelMatrixBufferDescriptorInfo.offset = 0;
	modelMatrixBufferDescriptorInfo.range = modelMatrixBufferSize;


	for_each(renderables.begin(), renderables.end(), [](const auto&v) {
		v->material->update();
		});

	LOGI("�������㻺��");
	{
		vk::CommandBuffer cmdBuffVert = beginSingleTimeCommands();
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		Buffer stagingBuffer = Buffer::createStagingBuffer(vertices.data(), bufferSize,
			*vmaAllocator);

		vertBuff.create(*vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(cmdBuffVert, stagingBuffer.buffer, vertBuff.buffer, 1, &copyRegion);
		endSingleTimeCommands(cmdBuffVert);
	}

	LOGI("������������");
	{
		vk::CommandBuffer cmdBuffIndex = beginSingleTimeCommands();
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		Buffer stagingBuffer = Buffer::createStagingBuffer(indices.data(), bufferSize,
			*vmaAllocator);

		indexBuffer.create(*vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(cmdBuffIndex, stagingBuffer.buffer, indexBuffer.buffer, 1, &copyRegion);
		endSingleTimeCommands(cmdBuffIndex);
	}

	LOGI("������ӻ��ƻ���");
	{
		bufferIndirect.create(*vmaAllocator, draws.size() * sizeof(vk::DrawIndexedIndirectCommand),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(*vmaAllocator, bufferIndirect.bufferMemory, &allocInfo);
		auto bufferIndirectP = static_cast<vk::DrawIndexedIndirectCommand*>(allocInfo.pMappedData);
		for (uint32_t i = 0, firstIndex = 0; i < draws.size(); ++i)
		{
			bufferIndirectP[i].firstIndex = firstIndex;
			bufferIndirectP[i].firstInstance = draws[i].first;
			bufferIndirectP[i].indexCount = renderables[draws[i].first]->mesh->indices.size();
			bufferIndirectP[i].instanceCount = 0;
			bufferIndirectP[i].vertexOffset = 0;

			firstIndex += bufferIndirectP[i].indexCount;
		}
		descriptorBufferInfoIndirect.buffer = bufferIndirect.buffer;
		descriptorBufferInfoIndirect.range = bufferIndirect.size;
	}

	device->updateDescriptorSets(materialCull->writeDescriptorSets.size(),
		materialCull->writeDescriptorSets.data(), 0, nullptr);

	// record command
	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBuffersCompute[0]->begin(commandBufferBeginInfo);

	commandBuffersCompute[0]->bindPipeline(vk::PipelineBindPoint::eCompute, materialCull->pipelinePass->pipeline.get());
	commandBuffersCompute[0]->bindDescriptorSets(vk::PipelineBindPoint::eCompute,
		materialCull->pipelinePass->pipelineLayout.get(), 0, materialCull->descriptorSets.size(), materialCull->descriptorSets.data(), 0, nullptr);

	commandBuffersCompute[0]->dispatch(renderables.size() / 1024+1, 1, 1);

	commandBuffersCompute[0]->end();
}

void Crane::Render::draw()
{
	commandBuffer[currBuffIndex]->reset(vk::CommandBufferResetFlagBits{});
	vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBuffer[currBuffIndex]->begin(commandBufferBeginInfo);

	if (currBuffIndex == 0)
	{
		TracyVkZone(tracyVkCtx, commandBuffer[currBuffIndex].get(), "All Frame");
	}

	vk::RenderPassBeginInfo rpBeginInfo{
		.renderPass = renderPass.get(),
		.framebuffer = framebuffers[currBuffIndex].get(),
		.renderArea = {.offset = {.x = 0, .y = 0},
						.extent = {.width = width, .height = height}},
		.clearValueCount = 2,
		.pClearValues = clearValues };
	commandBuffer[currBuffIndex]->beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);

	// begin
	{
		vk::Pipeline pipelineLast = nullptr;

		commandBuffer[currBuffIndex]->bindVertexBuffers(uint32_t(0), 1, (vk::Buffer*)&vertBuff.buffer, vertOffsets);
		commandBuffer[currBuffIndex]->bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
		for (uint32_t i = 0; i < draws.size(); ++i)
		{
			IndirectBatch& draw = draws[i];
			vk::Pipeline pipelineNew = draw.renderable->material->pipelinePass->pipeline.get();
			vk::PipelineLayout pipelineLayout = draw.renderable->material->pipelinePass->pipelineLayout.get();
			vk::DescriptorSet* descriptorSetP = draw.renderable->material->descriptorSets.data();
			uint32_t descriptorSetCount = draw.renderable->material->descriptorSets.size();
			//MeshBase *meshNew = draw.renderable->mesh;
			if (pipelineNew != pipelineLast)
			{
				commandBuffer[currBuffIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineNew);

				commandBuffer[currBuffIndex]->pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
					0, sizeof(cameraPushConstants[currBuffIndex]), &cameraPushConstants[currBuffIndex]);

				pipelineLast = pipelineNew;
			}

			vector<uint32_t> offsets{ sceneParametersUniformOffset * currBuffIndex };
			commandBuffer[currBuffIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSetCount, descriptorSetP, offsets.size(), offsets.data());
			VkDeviceSize offsetIndirect = i * sizeof(vk::DrawIndexedIndirectCommand);
			commandBuffer[currBuffIndex]->drawIndexedIndirect(bufferIndirect.buffer, offsetIndirect, 1, sizeof(vk::DrawIndexedIndirectCommand));
		}
	}
	// end

	commandBuffer[currBuffIndex]->endRenderPass();
	commandBuffer[currBuffIndex]->end();

	submitInfo.pCommandBuffers = &commandBuffer[currBuffIndex].get();
	submitInfo.pWaitSemaphores = &imageAcquiredSemaphores[currentFrame].get();
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame].get();

	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame].get();

	if (drawGUIFlag) // drawGUIFlag
	{
		graphicsQueue.submit(submitInfo);
		drawGUI();
	}
	else
	{
		graphicsQueue.submit(submitInfo, inFlightFences[currentFrame].get());
	}
}

void Crane::Render::update()
{

	device->waitForFences(inFlightFences[currentFrame].get(), true, UINT64_MAX);
	currBuffIndex = device->acquireNextImageKHR(swapchain.get(), UINT64_MAX, imageAcquiredSemaphores[currentFrame].get());
	device->waitForFences(imagesInFlightFences[currBuffIndex], true, UINT64_MAX);
	imagesInFlightFences[currBuffIndex] = inFlightFences[currentFrame].get();
	device->resetFences(inFlightFences[currentFrame].get());

	updateApp();

	updateCameraBuffer();
	updateSceneParameters();

	if (!renderables.empty())
	{
		auto modelMatrixPtr = modelMatrix.data();
		for (auto &v : renderables)
		{
			*(reinterpret_cast<Eigen::Matrix4f *>(modelMatrixPtr)) = v->transformer.getTransformWorld();
			modelMatrixPtr += modelMatrixOffset;
		}
		modelMatrixBuffer.update(modelMatrix.data());
	}

	updateCullData();

	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(*vmaAllocator, bufferIndirect.bufferMemory, &allocInfo);
	auto bufferIndirectP = static_cast<vk::DrawIndexedIndirectCommand*>(allocInfo.pMappedData);
	for (uint32_t i = 0, firstIndex = 0; i < draws.size(); ++i)
	{
		bufferIndirectP[i].instanceCount = 0;
	}

	computeQueue.submit(1, &submitInfoCompute, vk::Fence{});
	computeQueue.waitIdle();

	draw();

	//present
	presentInfo.pImageIndices = &currBuffIndex;
	presentQueue.presentKHR(presentInfo);
	currentFrame = (currentFrame + 1) % swapchainImages.size();
}

void Crane::Render::updateCameraBuffer()
{
	Eigen::Vector4f position;
	position.head(3) = camera.getCameraPos();
	position[3] = 1.0f;
	cameraPushConstants[currBuffIndex].position = position;
	cameraPushConstants[currBuffIndex].projView = camera.projection * camera.view;
}

void Crane::Render::updateSceneParameters()
{
	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(*vmaAllocator, sceneParameterBuffer.bufferMemory, &allocInfo);
	memcpy(static_cast<uint8_t*>(allocInfo.pMappedData) +
		(sceneParametersUniformOffset * currBuffIndex),
		&sceneParameters, sizeof(SceneParameters));
}
