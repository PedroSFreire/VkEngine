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
		logicalDevice = other.logicalDevice;
		other.semaphore = VK_NULL_HANDLE;
		other.logicalDevice = nullptr;
	}

	VulkanSemaphore & operator=(VulkanSemaphore&& other) noexcept {
		if (this != &other) {
			semaphore = other.semaphore;
			logicalDevice = other.logicalDevice;
			other.semaphore = VK_NULL_HANDLE;
			other.logicalDevice = nullptr;
		}
		return *this;
	}

	VkSemaphore getSemaphore() const { return semaphore; }
	const VkSemaphore* getSemaphorePtr() const { return &semaphore; }



private:
	VkSemaphore semaphore;
	const VulkanLogicalDevice* logicalDevice = NULL;
};

