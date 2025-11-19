#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <stdexcept>

class Window;
class VulkanInstance;

class VulkanSurface
{

public:
	VulkanSurface() = default;
	VulkanSurface(const VulkanSurface&) = delete;
	~VulkanSurface();

	VkSurfaceKHR getSurface() const { return surface; }

	void createSurface(const VulkanInstance& instance, const Window& window);

private:
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	const VulkanInstance* instanceRef = NULL;

};

