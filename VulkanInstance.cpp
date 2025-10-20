#include "VulkanInstance.h"



VulkanInstance::VulkanInstance() :debugHandler() {}


void VulkanInstance::vulkanInstanceCreator() {
    createVkInstance();

    debugHandler.setupDebugMessenger(instance);
}




VulkanInstance::~VulkanInstance() {
    debugHandler.DestroyDebugUtilsMessengerEXT(instance, nullptr);

    vkDestroyInstance(instance, nullptr);
}
VkInstance& const VulkanInstance::getInstance() {
    return instance;
}

::VulkanDebugHandler& const VulkanInstance::getDebugHandler() {
    return debugHandler;
}






bool VulkanInstance::checkExtensions() {
	std::cout << "Checking extensions and layers availability...\n" << std::endl;
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
        extensions.data());

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : debugHandler.getValidationLayers()) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void VulkanInstance::createVkInstance() {
	std::cout << "Creating Vulkan instance...\n" << std::endl;  
    if (enableValidationLayers && !checkExtensions()) {
        throw std::runtime_error(
            "validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(debugHandler.validationLayers.size());
        createInfo.ppEnabledLayerNames = debugHandler.validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    auto extensions = debugHandler.getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}