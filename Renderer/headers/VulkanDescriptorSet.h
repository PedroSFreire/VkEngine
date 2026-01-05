#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <chrono>


class VulkanLogicalDevice;
class VulkanImageView;
class VulkanBuffer;
class VulkanSampler;
class VulkanDescriptorPool;




class VulkanDescriptorSet
{
private:
	VkDescriptorSetLayout descriptorSetLayout{};
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkDescriptorPool* descriptorPool = nullptr;

	const VulkanLogicalDevice* logicalDevice = NULL;

public:

	VulkanDescriptorSet() = default;
	VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
	~VulkanDescriptorSet();
	VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept {

		descriptorSetLayout = other.descriptorSetLayout;
		descriptorSet = other.descriptorSet;
		descriptorPool = other.descriptorPool;
		logicalDevice = other.logicalDevice;

		other.descriptorSetLayout = VK_NULL_HANDLE;
		other.descriptorPool = VK_NULL_HANDLE;
		other.descriptorPool = nullptr;
		other.logicalDevice = nullptr;

	}

	VkDescriptorSetLayout getDescriptorSetLayout() const{ return descriptorSetLayout; }
	VkDescriptorSet getDescriptorSet()const  { return descriptorSet; }


	const VkDescriptorPool* getDescriptorPool() const { return descriptorPool; }
	//void createDescriptorSetLayoutNew (VulkanLogicalDevice& device, VulkanDescriptorPool& pool);
	//void createDescriptorSetLayout(VulkanLogicalDevice& device, VulkanDescriptorPool& pool);
	//void createDescriptorSetNew(VulkanBuffer& uniformBuffer, const VulkanImageView& textureView, const VulkanSampler& textureSampler);
	//void updateDescriptorSetNew(VulkanBuffer& uniformBuffer, const VulkanImageView& textureView, const VulkanSampler& textureSampler);

	void createMaterialDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool);
	void createUBODescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool);
	void createLightDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool);

	VkDescriptorSetLayout createMaterialDescriptorLayout(const VulkanLogicalDevice& device);
	VkDescriptorSetLayout createLightDescriptorLayout(const VulkanLogicalDevice& device);

	void createDescriptor();

	void updateLightDescriptor(VulkanBuffer& lightBuffer, size_t numLights);
	void updateMaterialDescriptor(const VkImageView* textureView, const VkSampler* textureSampler);
	void updateUBODescriptor(VulkanBuffer& uniformBuffer);

	//void createDescriptorSet(std::vector<VulkanBuffer>& uniformBuffers, int size,const VulkanImageView& textureView,const VulkanSampler& textureSampler);

};

