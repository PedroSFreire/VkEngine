#include "..\headers\VulkanSurface.h"
#include "..\headers\Window.h"
#include "..\headers\VulkanInstance.h"






VulkanSurface::~VulkanSurface() {
    vkDestroySurfaceKHR((*instanceRef).getInstance(), surface, nullptr);
}


void VulkanSurface::createSurface(const VulkanInstance& instance, const Window& window) {
	instanceRef = &instance;
    if (glfwCreateWindowSurface(instanceRef->getInstance(), window.getWindow(), nullptr, &surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
