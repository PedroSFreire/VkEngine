#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include"VulkanPhysicalDevice.h"

const std::vector<const char*> deviceExtensions = {
     VK_KHR_SWAPCHAIN_EXTENSION_NAME };



class VulkanLogicalDevice
{
private:
    VkQueue graphicsQueue;

    VkQueue presentQueue;
	VkDevice device;
public:
	VulkanLogicalDevice() = default;
	~VulkanLogicalDevice();
	VulkanLogicalDevice(const VulkanLogicalDevice& other) = delete;

	VkDevice& getDevice() { return device; }
	VkQueue& getGraphicsQueue() { return graphicsQueue; }
	VkQueue& getPresentQueue() { return presentQueue; }
	void createLogicalDevice(VulkanInstance& instance, VulkanPhysicalDevice& physicalDevice, VulkanSurface& surface);

};

