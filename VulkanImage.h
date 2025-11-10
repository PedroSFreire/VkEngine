#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanBuffer.h"


class VulkanImage
{
private:
	VkImage image;
	VkDeviceMemory imageMemory;
	VulkanLogicalDevice* logicalDevice = nullptr;
	VkImageCreateInfo imageInfo{};
public:
	VulkanImage() = default;
	VulkanImage(const VulkanImage&) = delete;
	~VulkanImage();
	VulkanImage(VulkanImage&& other) noexcept {
		image = other.image;
		imageMemory = other.imageMemory;
		logicalDevice = other.logicalDevice;
		other.image = VK_NULL_HANDLE;
		other.imageMemory = VK_NULL_HANDLE;
		other.logicalDevice = nullptr;
	}
	void create2DImage(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& logicalDevice, int texWidth, int texHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
	VkImage& getImage() { return image; }
	VkImageCreateInfo getImageInfo() { return imageInfo; }
	VkDeviceMemory& getImageMemory() { return imageMemory; }
};

