#include "CommandBufferRecorder.h"



void CommandBufferRecorder::bindMesh(VulkanCommandBuffer& commandBuffer, const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer) {
	VkBuffer vertexBuffers[] = { vertBuffer.getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer.getCommandBuffer(), 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer.getCommandBuffer(), indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);


}

void CommandBufferRecorder::recordDrawCall(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const VulkanPipeline& graphicsPipeline, const CPUDrawCallData data, ResourceManager& resourceManager, VkDescriptorSet currentUBO) {
	std::array<VkDescriptorSet, 3> descriptor{ currentUBO,resourceManager.getDescriptor(data.mat->resourceId).getDescriptorSet() ,resourceManager.getLightDescriptor().getDescriptorSet() };



	const pushConstants pushData{ data.transform ,data.mat->colorFactor,data.mat->metallicFactor,data.mat->roughnessFactor,data.mat->emissiveStrenght,0,data.mat->emissiveFactor,0 };

	vkCmdPushConstants(commandBuffer.getCommandBuffer(), graphicsPipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushData), &pushData);

	vkCmdBindDescriptorSets(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getPipelineLayout(), 0, 3, descriptor.data(), 0, nullptr);

	vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), data.count, 1, data.startIndex, 0, 0);
}

void CommandBufferRecorder::recordCommandBufferBase(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const uint32_t imageIndex, SceneFramesData& drawData, VkDescriptorSet currentUBO, ResourceManager& resourceManager) {


	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;                   // Optional
	beginInfo.pInheritanceInfo = nullptr;  // Optional

	if (vkBeginCommandBuffer(commandBuffer.getCommandBuffer(), &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderer.getRenderPass().getRenderPass();
	renderPassInfo.framebuffer = renderer.getFrameBuffers().getFrameBufferAtIndex(imageIndex);
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = renderer.getSwapChain().getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();




	vkCmdBeginRenderPass(commandBuffer.getCommandBuffer(), &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
		renderer.getGraphicsPipeline().getPipeline());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(renderer.getSwapChain().getSwapChainExtent().width);
	viewport.height = static_cast<float>(renderer.getSwapChain().getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer.getCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = renderer.getSwapChain().getSwapChainExtent();
	vkCmdSetScissor(commandBuffer.getCommandBuffer(), 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.getGraphicsPipeline().getPipeline());



	for (CPUDrawCallInstanceData& instance : drawData.drawInstances) {
		if (instance.meshResourceId == -1) continue;
		MeshBuffers& mesh = resourceManager.getMesh(instance.meshResourceId).meshBuffers;
		bindMesh(commandBuffer, mesh.vertexBuffer, mesh.indexBuffer);

		for (CPUDrawCallData& drawCall : instance.cpuDrawCalls) {

			recordDrawCall(renderer,commandBuffer, renderer.getGraphicsPipeline(), drawCall, resourceManager, currentUBO);
		}
	}


	vkCmdEndRenderPass(commandBuffer.getCommandBuffer());

	if (vkEndCommandBuffer(commandBuffer.getCommandBuffer()) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void CommandBufferRecorder::recordCommandBufferCopyBuffer(VulkanCommandBuffer& commandBuffer, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize size) {

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer.getCommandBuffer(), &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer.getCommandBuffer());

}

