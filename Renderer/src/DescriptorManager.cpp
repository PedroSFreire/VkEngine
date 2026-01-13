#include "..\headers\DescriptorManager.h"
#include "..\headers\VulkanRenderer.h"


void DescriptorManager::createMaterialDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorSet& set, VulkanDescriptorPool& pool)  {
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

	set.createDescriptorLayout(device, pool, bindings);
}
void DescriptorManager::createUBODescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorSet& set, VulkanDescriptorPool& pool)  {


	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	set.createDescriptorLayout(device, pool, bindings);
}
void DescriptorManager::createLightDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorSet& set, VulkanDescriptorPool& pool)  {

	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	set.createDescriptorLayout(device, pool, bindings);
}

void DescriptorManager::createMaterialDescriptorPool(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, int size)  {
	DescriptorPoolCreateInfo info{};

	std::array<VkDescriptorPoolSize, 1> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size) * 5;

	info.poolSizes = poolSizes;
	info.maxSets = static_cast<uint32_t>(size);
	info.flags = 0;

	pool.createDescriptorPool(device, info);
}
void DescriptorManager::createUBODescriptorPool(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, int size)  {

	DescriptorPoolCreateInfo info{};

	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size);

	info.poolSizes = poolSizes;
	info.maxSets = static_cast<uint32_t>(size);
	info.flags = 0;

	pool.createDescriptorPool(device, info);
}
void DescriptorManager::createLightDescriptorPool(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, size_t size)  {
	DescriptorPoolCreateInfo info{};

	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size);

	info.poolSizes = poolSizes;
	info.maxSets = static_cast<uint32_t>(size);
	info.flags = 0;

	pool.createDescriptorPool(device, info);
}

void DescriptorManager::updateLightDescriptor( VulkanDescriptorSet& set, VulkanBuffer& lightBuffer, size_t numLights)  {
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = lightBuffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;


	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = set.getDescriptorSet();
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	set.updateDescriptorSet(descriptorWrites);
}
void DescriptorManager::updateMaterialDescriptor( VulkanDescriptorSet& set, const VkImageView* textureView, const VkSampler* textureSampler)  {

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
	descriptorWrites[0].dstSet = set.getDescriptorSet();
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &colorInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = set.getDescriptorSet();
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &normalInfo;


	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = set.getDescriptorSet();
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &metalRoughInfo;


	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].dstSet = set.getDescriptorSet();
	descriptorWrites[3].dstBinding = 3;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].pImageInfo = &occlusionInfo;


	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].dstSet = set.getDescriptorSet();
	descriptorWrites[4].dstBinding = 4;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].pImageInfo = &emissiveInfo;

	set.updateDescriptorSet(descriptorWrites);
}
void DescriptorManager::updateUBODescriptor( VulkanDescriptorSet& set, VulkanBuffer& uniformBuffer)  {
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);


	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = set.getDescriptorSet();
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	set.updateDescriptorSet(descriptorWrites);
}


