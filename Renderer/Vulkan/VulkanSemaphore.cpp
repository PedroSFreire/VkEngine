#include "VulkanSemaphore.h"
#include "VulkanLogicalDevice.h"
#include <stdexcept>

VulkanSemaphore::VulkanSemaphore(const VulkanLogicalDevice& device)
{
	logicalDevice = &device;

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
		throw std::runtime_error("failed to create semaphore!");
	}
}


VulkanSemaphore::~VulkanSemaphore()
{
	if (semaphore != VK_NULL_HANDLE) {
		vkDestroySemaphore(logicalDevice->getDevice(), semaphore, nullptr);
	}
}