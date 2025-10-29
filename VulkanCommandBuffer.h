#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffers.h"
#include "VulkanGraphicsPipeline.h"


class VulkanCommandBuffer
{
private:

	VkCommandPool commandPool{};
	std::vector<VkCommandBuffer> commandBuffers;

	VulkanLogicalDevice* logicalDevice = NULL;
	int framesInFlight = 0;

public:
	VulkanCommandBuffer() = default;
	~VulkanCommandBuffer();
	VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;

	VkCommandBuffer& getCommandBuffer(int frameId) { return commandBuffers[frameId]; }

	void recordCommandBuffer(uint32_t imageIndex, int frameId, VulkanLogicalDevice& logicalDevice, VulkanSwapChain& swapChain, VulkanGraphicsPipeline& graphicsPipeline, VulkanRenderPass& renderPass, VulkanFrameBuffers& frameBuffers);


	void createCommandBuffer(VulkanLogicalDevice& device, const int MAX_FRAMES_IN_FLIGHT);


	void createCommandPool(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface);
};

