#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <stdexcept>

class VulkanLogicalDevice;
class VulkanImage;

class VulkanImageView
{
	private:
	VkImageView imageView;
	VulkanLogicalDevice* logicalDevice = nullptr;
public:
	VulkanImageView() = default;
	VulkanImageView(const VulkanImageView&) = delete;
	~VulkanImageView();
	VulkanImageView(VulkanImageView&& other) noexcept {
		imageView = other.imageView;
		logicalDevice = other.logicalDevice;
		other.imageView = VK_NULL_HANDLE;
		other.logicalDevice = nullptr;
	}
	void createImageView(VulkanLogicalDevice& logicalDevice, VulkanImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkImageView& getImageView() { return imageView; }
};

