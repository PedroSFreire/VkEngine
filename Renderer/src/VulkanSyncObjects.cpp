#include "..\headers\VulkanSyncObjects.h"
#include "..\headers\VulkanLogicalDevice.h"




VulkanSyncObjects::~VulkanSyncObjects() {

	for (size_t i = 0; i < framesInFlight; i++) {
		vkDestroySemaphore((*logicalDevice).getDevice(), imageAvailableSemaphores[i], nullptr);
		vkDestroyFence((*logicalDevice).getDevice(), inFlightFences[i], nullptr);
	}
	for (size_t i = 0; i < swapChainImagesCount; i++) {
		vkDestroySemaphore((*logicalDevice).getDevice(), renderFinishedSemaphores[i], nullptr);
	}

}




void VulkanSyncObjects::createSyncObjects(const VulkanLogicalDevice& device, size_t maxFramesInFlight, size_t nSwapChainImages) {

	logicalDevice = &device;
	framesInFlight = maxFramesInFlight;
	swapChainImagesCount = nSwapChainImages;

	imageAvailableSemaphores.resize(maxFramesInFlight);
	renderFinishedSemaphores.resize(nSwapChainImages);
	inFlightFences.resize(maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < maxFramesInFlight; i++) {
		if (vkCreateSemaphore((*logicalDevice).getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence((*logicalDevice).getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
	for (size_t i = 0; i < nSwapChainImages; i++) {
		if (vkCreateSemaphore((*logicalDevice).getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}