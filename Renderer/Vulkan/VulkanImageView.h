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

	VulkanImageView& operator=(VulkanImageView&& other) noexcept {
		if (this != &other) {
			clean();
			imageView = other.imageView;
			logicalDevice = other.logicalDevice;
			other.imageView = VK_NULL_HANDLE;
			other.logicalDevice = nullptr;
		}
		return *this;
	}

	void createImageView(const VulkanLogicalDevice& logicalDevice, const VulkanImage& image, const VkFormat format, const VkImageAspectFlags aspectFlags);
	
	void createImageView(const VulkanLogicalDevice& logicalDevice, const VulkanImage& image,int layer, const VkFormat format, const VkImageAspectFlags aspectFlags, uint32_t mipLevel = 0);

	void createCubeImageView(const VulkanLogicalDevice& logicalDevice, const VulkanImage& image, const VkFormat format, const VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);

	void createCubeImageLayerView(const VulkanLogicalDevice& logicalDevice, const VulkanImage& image, const VkFormat format, const VkImageAspectFlags aspectFlags, uint32_t miplevel);

	const VkImageView& getImageView() const { return imageView; }

	const VkImageView* getImageViewPtr()  const{ return &imageView; }


	void clean();


private:
	VkImageView imageView{};

	const VulkanLogicalDevice* logicalDevice = nullptr;

};

