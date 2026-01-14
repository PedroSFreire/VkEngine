#include "VulkanDescriptorSet.h"
#include "../Includes/VulkanIncludes.h"


VulkanDescriptorSet::~VulkanDescriptorSet() {

	if(logicalDevice != NULL)
	vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);
}


void VulkanDescriptorSet::createDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, std::span<const VkDescriptorSetLayoutBinding> bindings) {
	logicalDevice = &device;
	descriptorPool = &pool.getDescriptorPool();


	if (descriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);


	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void VulkanDescriptorSet::updateDescriptorSet(std::span<VkWriteDescriptorSet> data) {


	vkUpdateDescriptorSets(logicalDevice->getDevice(), static_cast<uint32_t>(data.size()), data.data(), 0, nullptr);
}

void VulkanDescriptorSet::createDescriptor() {
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = *descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(logicalDevice->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

