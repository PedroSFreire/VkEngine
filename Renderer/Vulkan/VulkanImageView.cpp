#include "VulkanImageView.h"
#include "../Includes/VulkanIncludes.h"


VulkanImageView::~VulkanImageView() {
	clean();
}


void VulkanImageView::clean() {
	if(!logicalDevice)
		return;
	if (logicalDevice) {
		vkDestroyImageView(logicalDevice->getDevice(), imageView, nullptr);
		imageView = VK_NULL_HANDLE;
		logicalDevice = nullptr;
	}
}

void VulkanImageView::createImageView(const VulkanLogicalDevice& logicalDevice, const VulkanImage& image, const VkFormat format, const VkImageAspectFlags aspectFlags) {
	this->logicalDevice = &logicalDevice;

	VkImageCreateInfo imageInfo = image.getImageInfo();

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image.getImage();
	if(imageInfo.imageType & VK_IMAGE_TYPE_3D) {
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else if(imageInfo.imageType & VK_IMAGE_TYPE_2D) {
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else if (imageInfo.imageType & VK_IMAGE_TYPE_1D) {
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	else {
		throw std::runtime_error("unsupported image type for image view!");
	}
	viewInfo.format = imageInfo.format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(logicalDevice.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

}
