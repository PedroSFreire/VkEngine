#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <chrono>
#include <span>


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


	void createDescriptor();

	void createDescriptorLayout(const VulkanLogicalDevice& device, VulkanDescriptorPool& pool, std::span<const VkDescriptorSetLayoutBinding> bindings);
	void updateDescriptorSet(std::span<VkWriteDescriptorSet> data);

	
};

