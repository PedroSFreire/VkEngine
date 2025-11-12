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
private:

	VkCommandBuffer commandBuffer;


public:
	VulkanCommandBuffer() = default;
	~VulkanCommandBuffer() = default;
	VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept {
		commandBuffer = other.commandBuffer;
		other.commandBuffer = VK_NULL_HANDLE;
	}


	



	inline VkCommandBuffer& getCommandBuffer() { return commandBuffer; }


	void recordCommandBuffer(uint32_t imageIndex, VulkanLogicalDevice& logicalDevice, VulkanSwapChain& swapChain, VulkanGraphicsPipeline& graphicsPipeline, VulkanRenderPass& renderPass, VulkanFrameBuffers& frameBuffers, VulkanBuffer& vertBuffer, VulkanBuffer& indexBuffer, VkDescriptorSet& descriptorSet);


	void createCommandBuffer(VulkanLogicalDevice& device, VulkanCommandPool& commandPool);

	
	void recordCommandBufferCopyBuffer(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VkBuffer&  srcBuffer, VkBuffer&   dstBuffer, VkDeviceSize size);



	void beginRecordindSingleTimeCommands(VulkanLogicalDevice& device, VulkanCommandPool& commandPool);

	void endRecordingSingleTimeCommands(VulkanLogicalDevice& device,  VulkanCommandPool& commandPool);

};

