#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Window.h"
#include "VulkanInstance.h"

#include <stdexcept>


class VulkanSurface
{
private:
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VulkanInstance* instanceRef = NULL;
public:
	VulkanSurface() = default;
	VulkanSurface(const VulkanSurface&) = delete;
	~VulkanSurface();

	VkSurfaceKHR& getSurface() { return surface; }

	void createSurface(VulkanInstance& instance, Window& window);


};

