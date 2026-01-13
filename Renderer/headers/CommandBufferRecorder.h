#pragma once
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderer.h"
#include "ResourceManager.h"
#include "scene.h"
#include "defines.h"


class CommandBufferRecorder {

public:
	CommandBufferRecorder() = default;
	CommandBufferRecorder(const CommandBufferRecorder&) = delete;
	~CommandBufferRecorder();


	static void bindMesh(VulkanCommandBuffer& commandBuffer, const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer);

	static void recordDrawCall(VulkanRenderer& renderer,VulkanCommandBuffer& commandBuffer, const VulkanPipeline& graphicsPipeline, const CPUDrawCallData data,  ResourceManager& resourceManager, VkDescriptorSet currentUBO);

	static void recordCommandBufferBase(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const uint32_t imageIndex, SceneFramesData& drawData, VkDescriptorSet descriptorSet, ResourceManager& resourceManager);

	static void recordCommandBufferCopyBuffer(VulkanCommandBuffer& commandBuffer, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize size);


};