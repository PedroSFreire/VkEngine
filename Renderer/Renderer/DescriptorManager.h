#pragma once
#include "../Includes/VulkanIncludes.h"
#include "../../Engine/Core/defines.h"
class DescriptorManager {

public:

	DescriptorManager() = default;
	DescriptorManager(const DescriptorManager&) = delete;
	~DescriptorManager();




	// *******************************************************************************************************************************************
// Descriptor Pools
	static void createMaterialDescriptorPool(const VulkanLogicalDevice& device,VulkanDescriptorPool& pool, int size);
	static void createUBODescriptorPool(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, int size);
	static void createLightDescriptorPool(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, size_t size);


	// Descriptor Set Layouts
	static void createMaterialDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorSet& set, VulkanDescriptorPool& pool);
	static void createUBODescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorSet& set, VulkanDescriptorPool& pool);
	static void createLightDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorSet& set, VulkanDescriptorPool& pool);

	// Descriptor Set Updates
	static void updateLightDescriptor( VulkanDescriptorSet& set, VulkanBuffer& lightBuffer, size_t numLights);
	static void updateMaterialDescriptor( VulkanDescriptorSet& set, const VkImageView* textureView, const VkSampler* textureSampler);
	static void updateUBODescriptor( VulkanDescriptorSet& set, VulkanBuffer& uniformBuffer);
	// *******************************************************************************************************************************************
};