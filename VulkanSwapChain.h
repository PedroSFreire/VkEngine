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


public:
	VulkanSwapChain() = default;
	~VulkanSwapChain();
	VulkanSwapChain(const VulkanSwapChain& other) = delete;

	void clean();
	VkSwapchainKHR getSwapChain() const { return swapChain; }
	const VkFormat& getSwapChainImageFormat() const { return swapChainImageFormat; }
	const VkExtent2D& getSwapChainExtent() const { return swapChainExtent; }
	const std::vector<VkImage>& getSwapChainImages() const { return swapChainImages; }
	const std::vector<VkImageView>& getSwapChainImageViews() const { return swapChainImageViews; }


	void createSwapChain(const VulkanPhysicalDevice& physicalDevice, const VulkanLogicalDevice& device, const VulkanSurface& surface,Window& window);

	void createImageViews();
private:

	VkSwapchainKHR					swapChain{};

	VkFormat						swapChainImageFormat{};

	VkExtent2D						swapChainExtent{};

	std::vector<VkImage>			swapChainImages;

	std::vector<VkImageView>		swapChainImageViews;

	const VulkanLogicalDevice* logicalDevice = nullptr;



	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;

	VkExtent2D chooseSwapExtent(Window& window, const VkSurfaceCapabilitiesKHR& capabilities) const;

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;

};

