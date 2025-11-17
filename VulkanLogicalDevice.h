#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanPhysicalDevice;
class VulkanInstance;
class VulkanSurface;

const std::vector<const char*> deviceExtensions = {
     VK_KHR_SWAPCHAIN_EXTENSION_NAME };



class VulkanLogicalDevice
{

public:

	VulkanLogicalDevice() = default;
	~VulkanLogicalDevice();
	VulkanLogicalDevice(const VulkanLogicalDevice& other) = delete;

	const VkDevice& getDevice() const { return device; }

	const VkQueue& getGraphicsQueue() const { return graphicsQueue; }

	const VkQueue& getPresentQueue() const { return presentQueue; }

	const VkQueue& getTransferQueue() const { return transferQueue; }

	void createLogicalDevice(const VulkanInstance& instance, const VulkanPhysicalDevice& physicalDevice, const VulkanSurface& surface);


private:
	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkQueue transferQueue;

	VkDevice device;

};

