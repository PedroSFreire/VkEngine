#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

class VulkanCommandBuffer
{


public:
	VulkanCommandBuffer() = default;
	~VulkanCommandBuffer() = default;
	VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept {
		commandBuffer = other.commandBuffer;
		other.commandBuffer = VK_NULL_HANDLE;
	}


	



	const VkCommandBuffer& getCommandBuffer() const { return commandBuffer; }

	void bindMesh(const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer);

	void recordDrawCall(const VulkanGraphicsPipeline& graphicsPipeline, const VulkanBuffer& indexBuffer,
		 const VulkanDescriptorSet& descriptorSet,const MaterialResource* mat ,const glm::mat4 transform);

	void recordCommandBuffer(uint32_t imageIndex, const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain,
		const VulkanGraphicsPipeline& graphicsPipeline, const VulkanRenderPass& renderPass, const VulkanFrameBuffers& frameBuffers,
		const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer,  VkDescriptorSet* descriptorSet);

	void recordCommandBufferNew(const uint32_t imageIndex, const VulkanRenderer& renderer, const GltfLoader& scene, VkDescriptorSet descriptorSet);

	void createCommandBuffer(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);
	
	void recordCommandBufferCopyBuffer(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VkBuffer&  srcBuffer, const VkBuffer&   dstBuffer, const VkDeviceSize size);

	void beginRecordindSingleTimeCommands(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);

	void endRecordingSingleTimeCommands(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);

	void bindMeshBuffers(const VulkanBuffer& vertexBuffer, const VulkanBuffer& indexBuffer);

	void bindDescriptorSet(const VulkanRenderer& renderer, VkPipelineLayout& pipelineLayout, const VkDescriptorSet& descriptorSet, const VulkanBuffer& indexBuffer);
private:

	VkCommandBuffer commandBuffer;
	VkDescriptorSet currentUBO;
	
};

