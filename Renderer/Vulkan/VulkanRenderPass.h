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

	void createForwardRenderPass(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain);

	void createCubeRenderPass(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice);

	void createEnvRenderPass(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice, const VulkanSwapChain& swapChain);

	VkRenderPass getRenderPass() const { return renderPass; }


private:

	VkRenderPass renderPass;

	const VulkanLogicalDevice* logicalDevice;


};

