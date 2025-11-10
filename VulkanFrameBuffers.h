#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"


#include <vector>
class VulkanFrameBuffers
{
private:

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VulkanLogicalDevice* logicalDevice = NULL;
public:
	VulkanFrameBuffers() = default;
	~VulkanFrameBuffers();
	VulkanFrameBuffers(const VulkanFrameBuffers&) = delete;



	void clean();

	std::vector<VkFramebuffer>& getSwapChainFramebuffers() { return swapChainFramebuffers; }

	VkFramebuffer& getFrameBufferAtIndex(size_t index) { return swapChainFramebuffers[index]; }

	void createFramebuffers(VulkanLogicalDevice& device, VulkanSwapChain& swapChain, VulkanRenderPass& renderPass);
};

