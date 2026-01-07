#include "..\headers\VulkanDescriptorPool.h"
#include "..\headers\VulkanDescriptorSet.h"
#include "..\headers\VulkanLogicalDevice.h"
#include "..\headers\VulkanImageView.h"
#include "..\headers\VulkanBuffer.h"
#include "..\headers\VulkanSampler.h"
#include "..\headers\defines.h"


VulkanDescriptorPool::~VulkanDescriptorPool() {
	vkDestroyDescriptorPool(logicalDevice->getDevice(), descriptorPool, nullptr);
}


void VulkanDescriptorPool::createDescriptorPool(const VulkanLogicalDevice& device, const DescriptorPoolCreateInfo& info) {
	logicalDevice = &device;
	maxSets = info.maxSets;
	if (maxSets <= 0) {
		throw std::runtime_error("Descriptor Pool size must be greater than 0!");
	}





	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(info.poolSizes.size());
	poolInfo.pPoolSizes = info.poolSizes.data();
	poolInfo.maxSets = maxSets;
	poolInfo.flags = info.flags;

	if (vkCreateDescriptorPool(logicalDevice->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}



