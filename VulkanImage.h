#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanLogicalDevice;
class VulkanPhysicalDevice;



class VulkanImage
{

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

	void create2DImage(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice, const int texWidth, const int texHeight,
						const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties);

	VkImage getImage() const { return image; }

	VkDeviceMemory getImageMemory() const { return imageMemory; }

	VkImageCreateInfo getImageInfo() const { return imageInfo; }

	void clean();


private:

	VkImage image{};

	VkDeviceMemory imageMemory{};

	const VulkanLogicalDevice* logicalDevice = nullptr;

	VkImageCreateInfo imageInfo{};


};

