#include "VulkanFence.h"
#include "VulkanLogicalDevice.h"
#include <stdexcept>


VulkanFence::VulkanFence(const VulkanLogicalDevice& device)
{
	logicalDevice = &device;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to create fence!");
	}
}


VulkanFence::~VulkanFence()
{
	if (fence != VK_NULL_HANDLE) {
		vkDestroyFence(logicalDevice->getDevice(), fence, nullptr);
	}
}

void VulkanFence::reset() const {
	vkResetFences(logicalDevice->getDevice(), 1, &fence);
}

void VulkanFence::wait() const {
	vkWaitForFences(logicalDevice->getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
}