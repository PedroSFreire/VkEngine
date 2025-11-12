#include "VulkanSurface.h"
#include "Window.h"
#include "VulkanInstance.h"






VulkanSurface::~VulkanSurface() {
    vkDestroySurfaceKHR((*instanceRef).getInstance(), surface, nullptr);
}


void VulkanSurface::createSurface(VulkanInstance& instance, Window& window) {
	instanceRef = &instance;
    if (glfwCreateWindowSurface((*instanceRef).getInstance(), window.getWindow(), nullptr, &surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
