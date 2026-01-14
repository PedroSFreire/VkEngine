#include "VulkanSurface.h"
#include "../Includes/VulkanIncludes.h"
#include "../Platform/Window.h"






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
