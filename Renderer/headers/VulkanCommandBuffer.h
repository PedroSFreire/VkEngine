#pragma once


#include "defines.h"

#include "glm/glm.hpp"

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFrameBuffers;
class VulkanGraphicsPipeline;
class VulkanBuffer;
class VulkanCommandPool;
class VulkanRenderer;
class GltfLoader;
class VulkanDescriptorSet;
class Scene;
class ResourceManager;

class VulkanCommandBuffer
{
private:

	VkCommandBuffer commandBuffer;
	VkDescriptorSet currentUBO = VK_NULL_HANDLE;

public:
	VulkanCommandBuffer() = default;
	~VulkanCommandBuffer() = default;
	VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept {
		commandBuffer = other.commandBuffer;
		other.commandBuffer = VK_NULL_HANDLE;
	}


	



	const VkCommandBuffer& getCommandBuffer() const { return commandBuffer; }

	void bindLights(const LightGPUData* lightData);

	void bindMesh(const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer);

	void recordDrawCall(const VulkanGraphicsPipeline& graphicsPipeline, const CPUDrawCallData data, uint32_t indexCount, ResourceManager& resourceManager);

	void recordCommandBufferScene(const uint32_t imageIndex, const VulkanRenderer& renderer, Scene& scene, VkDescriptorSet descriptorSet, ResourceManager& resourceManager);

	void createCommandBuffer(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);
	
	void recordCommandBufferCopyBuffer(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VkBuffer&  srcBuffer, const VkBuffer&   dstBuffer, const VkDeviceSize size);

	void beginRecordindSingleTimeCommands(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);

	void endRecordingSingleTimeCommands(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);

	void bindDescriptorSet(const VulkanRenderer& renderer, VkPipelineLayout& pipelineLayout, const VkDescriptorSet& descriptorSet, const VulkanBuffer& indexBuffer);

	
};

