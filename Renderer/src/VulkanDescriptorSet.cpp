#include "..\headers\VulkanDescriptorSet.h"
#include "..\headers\VulkanDescriptorPool.h"
#include "..\headers\VulkanLogicalDevice.h"
#include "..\headers\VulkanImageView.h"
#include "..\headers\VulkanBuffer.h"
#include "..\headers\VulkanSampler.h"
#include "..\headers\defines.h"


VulkanDescriptorSet::~VulkanDescriptorSet() {

	if(logicalDevice != NULL)
	vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);
}


void VulkanDescriptorSet::createLightDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool) {
	logicalDevice = &device;
	descriptorPool = &pool.getDescriptorPool();


	if (descriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);

	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VkDescriptorSetLayout VulkanDescriptorSet::createLightDescriptorLayout(const VulkanLogicalDevice& device) {
	logicalDevice = &device;


	if (descriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);

	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
	return descriptorSetLayout;
}

void VulkanDescriptorSet::updateLightDescriptor(VulkanBuffer& lightBuffer, size_t numLights) {
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = lightBuffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;


	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(logicalDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanDescriptorSet::createMaterialDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool) {
	logicalDevice = &device;
	descriptorPool = &pool.getDescriptorPool();

	if (descriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);

	std::array<VkDescriptorSetLayoutBinding, 5> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].pImmutableSamplers = nullptr;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[3].pImmutableSamplers = nullptr;
	bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[4].pImmutableSamplers = nullptr;
	bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VkDescriptorSetLayout VulkanDescriptorSet::createMaterialDescriptorLayout(const VulkanLogicalDevice& device) {
	logicalDevice = &device;

	if (descriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(logicalDevice->getDevice(), descriptorSetLayout, nullptr);

	std::array<VkDescriptorSetLayoutBinding, 5> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].pImmutableSamplers = nullptr;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[3].pImmutableSamplers = nullptr;
	bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[4].pImmutableSamplers = nullptr;
	bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	return descriptorSetLayout;
}


void VulkanDescriptorSet::createUBODescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool) {
	logicalDevice = &device;
	descriptorPool = &pool.getDescriptorPool();

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;

	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	uboLayoutBinding.pImmutableSamplers = nullptr;

	

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void VulkanDescriptorSet::updateMaterialDescriptor(const VkImageView* textureView, const VkSampler* textureSampler) {


	VkDescriptorImageInfo colorInfo{};
	colorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorInfo.imageView = textureView[0];
	colorInfo.sampler = textureSampler[0];

	VkDescriptorImageInfo normalInfo{};
	normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalInfo.imageView = textureView[1];
	normalInfo.sampler = textureSampler[1];

	VkDescriptorImageInfo metalRoughInfo{};
	metalRoughInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	metalRoughInfo.imageView = textureView[2];
	metalRoughInfo.sampler = textureSampler[2];

	VkDescriptorImageInfo occlusionInfo{};
	occlusionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	occlusionInfo.imageView = textureView[3];
	occlusionInfo.sampler = textureSampler[3];

	VkDescriptorImageInfo emissiveInfo{};
	emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	emissiveInfo.imageView = textureView[4];
	emissiveInfo.sampler = textureSampler[4];



	std::array<VkWriteDescriptorSet, 5> descriptorWrites{};



	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &colorInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &normalInfo;


	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &metalRoughInfo;


	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].dstSet = descriptorSet;
	descriptorWrites[3].dstBinding = 3;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].pImageInfo = &occlusionInfo;


	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].dstSet = descriptorSet;
	descriptorWrites[4].dstBinding = 4;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].pImageInfo = &emissiveInfo;




	vkUpdateDescriptorSets(logicalDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}


void VulkanDescriptorSet::updateUBODescriptor(VulkanBuffer& uniformBuffer) {
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);


	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(logicalDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

