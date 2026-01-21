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
	


    public:
        VulkanPhysicalDevice() = default;
		~VulkanPhysicalDevice() = default;
        VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;


        void pickPhysicalDevice(const VulkanInstance& instance, const VulkanSurface& surface);

        VkPhysicalDevice getPhysicalDevice() const  { return physicalDevice; }

        QueueFamilyIndices findQueueFamilies(const VulkanSurface& surface) const { return findQueueFamilies(physicalDevice, surface); };

        QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device,const VulkanSurface& surface) const ;

        SwapChainSupportDetails querySwapChainSupport(const VulkanSurface& surface) const { return querySwapChainSupport(physicalDevice, surface); };

        SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device, const VulkanSurface& surface) const;

        VkFormat findSupportedFormat( const std::vector<VkFormat>& candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features) const;

        uint32_t findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const;

        VkFormat findDepthFormat() const;

		VkSampleCountFlagBits getMsaaSamples() const { return msaaSamples; }

        bool hasStencilComponent(const VkFormat format) const { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }

	private:

        VkPhysicalDevice                    physicalDevice = VK_NULL_HANDLE;

        const std::vector<const char*>      deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        bool checkDeviceExtensionSupport(const VkPhysicalDevice device) const;

        bool isDeviceSuitable(const VkPhysicalDevice device, const  VulkanSurface& surface) const ;
        
        VkSampleCountFlagBits getMaxUsableSampleCount();
        

        

};

