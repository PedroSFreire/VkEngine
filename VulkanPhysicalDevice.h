#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <set>
#include <string>
#include <stdexcept>
#include "VulkanInstance.h"

#include "VulkanSurface.h"



struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
    }
};


class VulkanPhysicalDevice
{
	private:
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        const std::vector<const char*> deviceExtensions = {
     VK_KHR_SWAPCHAIN_EXTENSION_NAME };


    public:
        VulkanPhysicalDevice() = default;
		~VulkanPhysicalDevice() = default;
        VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;

        void pickPhysicalDevice(VulkanInstance& instance, VulkanSurface& surface);
        VkPhysicalDevice&  getPhysicalDevice() { return physicalDevice; }
        QueueFamilyIndices findQueueFamilies( VulkanSurface& surface) { return findQueueFamilies(physicalDevice, surface); };
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,  VulkanSurface& surface);
        SwapChainSupportDetails querySwapChainSupport( VulkanSurface& surface) { return querySwapChainSupport(physicalDevice, surface); };
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,  VulkanSurface& surface);

	private:

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        bool isDeviceSuitable(VkPhysicalDevice device, VulkanSurface& surface);

        

        

        

};

