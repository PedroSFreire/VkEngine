#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanSurface;




class VulkanCommandPool
{

private:

	VkCommandPool commandPool{};

	uint32_t queueFamilyIndex = 0;
	VkQueue* queue = NULL;

	VulkanLogicalDevice* logicalDevice = NULL;

public:
	VulkanCommandPool() = default;
	~VulkanCommandPool();
	VulkanCommandPool(const VulkanCommandPool&) = delete;
	VulkanCommandPool(VulkanCommandPool&& other) noexcept {
		commandPool = other.commandPool;
		logicalDevice = other.logicalDevice;
		queue = other.queue;
		other.commandPool = VK_NULL_HANDLE;
		other.logicalDevice = nullptr;
		other.queue = nullptr;
	}


	void createGraphicsCommandPool(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface);

	void createTransferCommandPool(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface);

	inline VkCommandPool getCommandPool() { return commandPool; }
	VkQueue* getQueue() { return queue; }
};

