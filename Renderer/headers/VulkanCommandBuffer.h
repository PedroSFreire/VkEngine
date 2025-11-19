#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFrameBuffers;
class VulkanGraphicsPipeline;
class VulkanBuffer;
class VulkanCommandPool;



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


	void recordCommandBuffer(uint32_t imageIndex, const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain,
		const VulkanGraphicsPipeline& graphicsPipeline, const VulkanRenderPass& renderPass, const VulkanFrameBuffers& frameBuffers,
		const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer, const VkDescriptorSet& descriptorSet);

	void createCommandBuffer(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);
	
	void recordCommandBufferCopyBuffer(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VkBuffer&  srcBuffer, const VkBuffer&   dstBuffer, const VkDeviceSize size);

	void beginRecordindSingleTimeCommands(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);

	void endRecordingSingleTimeCommands(const VulkanLogicalDevice& device, const VulkanCommandPool& commandPool);


private:

	VkCommandBuffer commandBuffer;

};

