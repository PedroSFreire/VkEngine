#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "VulkanLogicalDevice.h"

class VulkanSyncObjects
{

public:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VulkanSyncObjects() = default;
	~VulkanSyncObjects();
	VulkanSyncObjects(const VulkanSyncObjects& other) = delete;


	void createSyncObjects(VulkanLogicalDevice& device, const int MAX_FRAMES_IN_FLIGHT);


private:
	VulkanLogicalDevice* logicalDevice;
	int framesInFlight = 0;
	
};

