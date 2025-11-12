#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <vector>
#include <algorithm>


class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanSurface;
class Window;

class VulkanSwapChain
{
private:

	
	
	VkSwapchainKHR					swapChain{};
	VkFormat						swapChainImageFormat{};
	VkExtent2D						swapChainExtent{};
	std::vector<VkImage>			swapChainImages;
	std::vector<VkImageView>		swapChainImageViews;

	//handle required for object destruction
	VulkanLogicalDevice* logicalDevice = NULL;

public:
	VulkanSwapChain() = default;
	~VulkanSwapChain();
	VulkanSwapChain(const VulkanSwapChain& other) = delete;

	void clean();
	VkSwapchainKHR& getSwapChain() { return swapChain; }
	VkFormat& getSwapChainImageFormat() { return swapChainImageFormat; }
	VkExtent2D& getSwapChainExtent() { return swapChainExtent; }
	std::vector<VkImage>& getSwapChainImages() { return swapChainImages; }
	std::vector<VkImageView>& getSwapChainImageViews() { return swapChainImageViews; }


	void createSwapChain(VulkanPhysicalDevice& physicalDevice, VulkanLogicalDevice& device, VulkanSurface& surface,Window& window);

	void createImageViews();
private:

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkExtent2D chooseSwapExtent(Window& window, const VkSurfaceCapabilitiesKHR& capabilities);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);


};

