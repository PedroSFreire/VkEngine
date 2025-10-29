#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanSwapChain.h"
#include "VulkanLogicalDevice.h"


class VulkanRenderPass
{
private:
	VkRenderPass renderPass;
	VulkanLogicalDevice* logicalDevice;

public:
	VulkanRenderPass() = default;
	VulkanRenderPass(const VulkanRenderPass&) = delete;
	~VulkanRenderPass();

	void createRenderPass(VulkanSwapChain& swapChain, VulkanLogicalDevice& logicalDevice);

	VkRenderPass& getRenderPass() { return renderPass; }


};

