#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include<vector>





#ifdef NDEBUG
inline const bool enableValidationLayers = false;
#else
inline const bool enableValidationLayers = true;
#endif

class VulkanDebugHandler
{

public:
    VkDebugUtilsMessengerEXT debugMessenger;
    static const std::vector<const char*> validationLayers;

public:
    VulkanDebugHandler() = default;

	VulkanDebugHandler(const VulkanDebugHandler&) = delete;


    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);

    std::vector<const char*> getRequiredExtensions();

    void setupDebugMessenger(VkInstance instance);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,  const VkAllocationCallbacks* pAllocator);

    const std::vector<const char*>& getValidationLayers() {return validationLayers;}

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;



        return VK_FALSE;
    }




};
