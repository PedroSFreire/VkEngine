#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanSurface;




class VulkanCommandPool
{


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


	void createGraphicsCommandPool(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VulkanSurface& surface);

	void createTransferCommandPool(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VulkanSurface& surface);

	VkCommandPool getCommandPool() const { return commandPool; }

	const VkQueue& getQueue() const { return *queue; }


private:

	VkCommandPool					commandPool{};

	uint32_t						queueFamilyIndex = 0;

	const VkQueue*					queue = NULL;

	const VulkanLogicalDevice*		logicalDevice = NULL;


};

