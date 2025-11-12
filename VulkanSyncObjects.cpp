#include "VulkanSyncObjects.h"
#include "VulkanLogicalDevice.h"




VulkanSyncObjects::~VulkanSyncObjects() {

	for (size_t i = 0; i < framesInFlight; i++) {
		vkDestroySemaphore((*logicalDevice).getDevice(), renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore((*logicalDevice).getDevice(), imageAvailableSemaphores[i], nullptr);
		vkDestroyFence((*logicalDevice).getDevice(), inFlightFences[i], nullptr);
	}
}




void VulkanSyncObjects::createSyncObjects(VulkanLogicalDevice& device, const int MAX_FRAMES_IN_FLIGHT) {

	logicalDevice = &device;
	framesInFlight = MAX_FRAMES_IN_FLIGHT;

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore((*logicalDevice).getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore((*logicalDevice).getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence((*logicalDevice).getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}