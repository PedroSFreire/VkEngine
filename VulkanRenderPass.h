#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>

class VulkanSwapChain;
class VulkanPhysicalDevice;
class VulkanLogicalDevice;



class VulkanRenderPass
{

public:
	VulkanRenderPass() = default;
	VulkanRenderPass(const VulkanRenderPass&) = delete;
	~VulkanRenderPass();

	void createRenderPass(const VulkanPhysicalDevice& physicalDevice , const VulkanSwapChain& swapChain, const VulkanLogicalDevice& logicalDevice);

	VkRenderPass getRenderPass() const { return renderPass; }


private:

	VkRenderPass renderPass;

	const VulkanLogicalDevice* logicalDevice;


};

