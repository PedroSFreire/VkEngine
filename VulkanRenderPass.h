#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>

class VulkanSwapChain;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;



class VulkanRenderPass
{
private:
	VkRenderPass renderPass;
	VulkanLogicalDevice* logicalDevice;

public:
	VulkanRenderPass() = default;
	VulkanRenderPass(const VulkanRenderPass&) = delete;
	~VulkanRenderPass();

	void createRenderPass(VulkanPhysicalDevice& physicalDevice , VulkanSwapChain& swapChain, VulkanLogicalDevice& logicalDevice);

	VkRenderPass& getRenderPass() { return renderPass; }


};

