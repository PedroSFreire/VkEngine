#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanLogicalDevice.h"
#include "VulkanImageView.h"
#include "VulkanBuffer.h"
#include <vector>
#include "VulkanSampler.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};


class VulkanDescriptorSet
{
private:
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorPool descriptorPool;

	VulkanLogicalDevice* logicalDevice = NULL;

public:

	VulkanDescriptorSet() = default;
	VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
	~VulkanDescriptorSet();
	VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept {

		descriptorSetLayout = other.descriptorSetLayout;
		descriptorSets = std::move(other.descriptorSets);
		descriptorPool = other.descriptorPool;
		logicalDevice = other.logicalDevice;

		other.descriptorSetLayout = VK_NULL_HANDLE;
		other.descriptorPool = VK_NULL_HANDLE;
		other.descriptorPool = nullptr;
		other.logicalDevice = nullptr;

	}

	VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout; }
	VkDescriptorSet& getDescriptorSets(int i) { return descriptorSets[i]; }
	VkDescriptorPool& getDescriptorPool() { return descriptorPool; }







	void createDescriptorSetLayout(VulkanLogicalDevice& device);

	void createDescriptorPool(int size);

	void createDescriptorSets(std::vector<VulkanBuffer>& uniformBuffers, int size, VulkanImageView& textureView, VulkanSampler& textureSampler);

};

