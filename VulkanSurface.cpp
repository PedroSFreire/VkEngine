#include "VulkanSurface.h"




VulkanSurface::VulkanSurface(VulkanInstance& instance): instanceRef( instance.getInstance()) {
    
}


VulkanSurface::~VulkanSurface() {
    vkDestroySurfaceKHR(instanceRef, surface, nullptr);
}


void VulkanSurface::createSurface(VulkanInstance& instance, Window& window) {
	
    if (glfwCreateWindowSurface(instance.getInstance(), window.getWindow(), nullptr, &surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
