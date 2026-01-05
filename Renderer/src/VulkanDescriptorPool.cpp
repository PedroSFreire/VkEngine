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







void VulkanDescriptorPool::createUBODescriptorPool( const VulkanLogicalDevice& device, int size) {

	logicalDevice = &device;
	maxSets = size;
	if (maxSets <= 0) {
		throw std::runtime_error("Descriptor Pool size must be greater than 0!");
	}

	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size);



	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(size);

	if (vkCreateDescriptorPool(logicalDevice->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

}


void VulkanDescriptorPool::createMaterialDescriptorPool( const VulkanLogicalDevice& device, int size) {

	logicalDevice = &device;
	maxSets = size;
	if (maxSets <= 0) {
		throw std::runtime_error("Descriptor Pool size must be greater than 0!");
	}

	std::array<VkDescriptorPoolSize, 1> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size)*5;



	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(size);

	if (vkCreateDescriptorPool(logicalDevice->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

}

void VulkanDescriptorPool::createLightDescriptorPool(const VulkanLogicalDevice& device, size_t size) {

	logicalDevice = &device;
	maxSets = size;
	if (maxSets <= 0) {
		throw std::runtime_error("Descriptor Pool size must be greater than 0!");
	}

	std::array<VkDescriptorPoolSize, 1> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = 1;



	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = size;

	if (vkCreateDescriptorPool(logicalDevice->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

}





