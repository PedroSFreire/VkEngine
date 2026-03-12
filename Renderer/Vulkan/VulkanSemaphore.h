#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanLogicalDevice;

class VulkanSemaphore
{
public:
	VulkanSemaphore(const VulkanLogicalDevice& device);
	VulkanSemaphore(const VulkanSemaphore&) = delete;
	~VulkanSemaphore();
	VulkanSemaphore(VulkanSemaphore&& other) noexcept {
		semaphore = other.semaphore;
		other.semaphore = VK_NULL_HANDLE;
	}

	VulkanSemaphore & operator=(VulkanSemaphore&& other) noexcept {
		if (this != &other) {
			semaphore = other.semaphore;
			other.semaphore = VK_NULL_HANDLE;
		}
		return *this;
	}

	VkSemaphore getSemaphore() const { return semaphore; }
	const VkSemaphore* getSemaphorePtr() const { return &semaphore; }



private:
	VkSemaphore semaphore;
	const VulkanLogicalDevice* logicalDevice = NULL;
};

