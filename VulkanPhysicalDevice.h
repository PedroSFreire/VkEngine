#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <set>
#include <string>
#include <stdexcept>

class VulkanInstance;
class VulkanSurface;



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
        VkFormat findSupportedFormat( const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkFormat findDepthFormat();

        bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }

	private:

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        bool isDeviceSuitable(VkPhysicalDevice device, VulkanSurface& surface);

        

        

        

};

