#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanLogicalDevice;
class VulkanPhysicalDevice;



class VulkanSampler
{

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
	void createTextureSampler(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& logicalDevice);
	VkSampler getSampler() const { return sampler; }

private:
	VkSampler sampler{};

	const VulkanLogicalDevice* logicalDevice = nullptr;


};

