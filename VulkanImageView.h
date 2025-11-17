#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <stdexcept>

class VulkanLogicalDevice;
class VulkanImage;

class VulkanImageView
{

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

	void createImageView(const VulkanLogicalDevice& logicalDevice, const VulkanImage& image, const VkFormat format, const VkImageAspectFlags aspectFlags);

	VkImageView getImageView() const { return imageView; }

	void clean();


private:
	VkImageView imageView{};

	const VulkanLogicalDevice* logicalDevice = nullptr;

};

