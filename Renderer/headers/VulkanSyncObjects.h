#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

class VulkanLogicalDevice;

class VulkanSyncObjects
{

public:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VulkanSyncObjects() = default;
	~VulkanSyncObjects();
	VulkanSyncObjects(const VulkanSyncObjects& other) = delete;


	void createSyncObjects(const VulkanLogicalDevice& device, size_t maxFramesInFlight , size_t nSwapChainImages);


private:

	const VulkanLogicalDevice* logicalDevice = NULL;

	int framesInFlight = 0;

	int swapChainImagesCount = 0;
	
};

