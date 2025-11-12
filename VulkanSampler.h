#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanLogicalDevice;
class VulkanPhysicalDevice;



class VulkanSampler
{

	private:
	VkSampler sampler;
	VulkanLogicalDevice* logicalDevice = nullptr;

public:
	VulkanSampler() = default;
	VulkanSampler(const VulkanSampler&) = delete;
	~VulkanSampler();
	VulkanSampler(VulkanSampler&& other) noexcept {
		sampler = other.sampler;
		logicalDevice = other.logicalDevice;
		other.sampler = VK_NULL_HANDLE;
		other.logicalDevice = nullptr;
	}
	void createTextureSampler(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& logicalDevice);
	VkSampler& getSampler() { return sampler; }
};

