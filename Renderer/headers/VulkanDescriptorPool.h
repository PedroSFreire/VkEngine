#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/*
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <chrono>
*/

class VulkanLogicalDevice;






class VulkanDescriptorPool
{
private:

	VkDescriptorPool descriptorPool{};

	 VulkanLogicalDevice* logicalDevice = NULL;

	int maxSets = 0;

public:

	VulkanDescriptorPool() = default;
	VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
	~VulkanDescriptorPool();
	VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept {


		descriptorPool = other.descriptorPool;
		logicalDevice = other.logicalDevice;

		other.descriptorPool = VK_NULL_HANDLE;
		other.descriptorPool = nullptr;
		other.logicalDevice = nullptr;

	}

	VkDescriptorPool& getDescriptorPool() { return descriptorPool; }


	void createMaterialDescriptorPool( VulkanLogicalDevice& device, int size);
	void createUBODescriptorPool( VulkanLogicalDevice& device, int size);

};

