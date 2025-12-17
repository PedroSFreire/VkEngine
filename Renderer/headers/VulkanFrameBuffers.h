#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>

class VulkanLogicalDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanImageView;


class VulkanFrameBuffers
{

public:
	VulkanFrameBuffers() = default;
	~VulkanFrameBuffers();
	VulkanFrameBuffers(const VulkanFrameBuffers&) = delete;



	void clean();

	const std::vector<VkFramebuffer>& getSwapChainFramebuffers() const { return swapChainFramebuffers; }

	const VkFramebuffer getFrameBufferAtIndex(size_t index) const { return swapChainFramebuffers[index]; }

	void createFramebuffers(const VulkanLogicalDevice& device, const VulkanSwapChain& swapChain, const VulkanRenderPass& renderPass , const VulkanImageView& depthImageView);

private:

	std::vector<VkFramebuffer> swapChainFramebuffers;

	const VulkanLogicalDevice* logicalDevice = NULL;
};

