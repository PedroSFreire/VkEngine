#include "VulkanImage.h"
#include "../Includes/VulkanIncludes.h"

#include <stdexcept>
#include <iostream>


VulkanImage::VulkanImage() {

}

VulkanImage::~VulkanImage() {
	clean();
}


void VulkanImage::clean() {
	if( allocatorHandle == nullptr) {
		return;
	}
	vmaDestroyImage(allocatorHandle->getAllocator(), image, allocation);
}


void VulkanImage::create2DImage(const VulkanMemoryAllocator& allocator, const VulkanImageCreateInfo& info) {
	allocatorHandle = &allocator;


	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.extent.width = info.width;
	imageInfo.extent.height = info.height;
	imageInfo.format = info.format;
	imageInfo.tiling = info.tiling;
	imageInfo.usage = info.usage;

	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = info.mipLevels;
	imageInfo.arrayLayers = info.layers;

	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = info.numSamples;
	imageInfo.flags = info.flags;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = info.vmaUsage;
	allocInfo.flags = info.vmaFlags;

	if (vmaCreateImage(allocator.getAllocator(), &imageInfo, &allocInfo, &image, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}


}




