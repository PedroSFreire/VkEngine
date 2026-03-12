#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanLogicalDevice;
class VulkanFence
{
public:
	VulkanFence(const VulkanLogicalDevice&);
	VulkanFence(const VulkanFence&) = delete;
	~VulkanFence();
	VulkanFence(VulkanFence&& other) {
		fence = other.fence;
		other.fence = VK_NULL_HANDLE;
	}

	VulkanFence& operator=(VulkanFence&& other) {
		if (this != &other) {
			fence = other.fence;
			other.fence = VK_NULL_HANDLE;
		}
		return *this;
	}

	VkFence getFence() const { return fence; }
	const VkFence* getFencePtr() const { return &fence; }

	void reset() const;

	void wait() const;


private:
	VkFence fence;
	const VulkanLogicalDevice* logicalDevice = NULL;
};