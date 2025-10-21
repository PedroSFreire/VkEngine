#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanDebugHandler.h" 
#include <iostream>


class VulkanInstance {
	private:
		
		VkInstance instance = VK_NULL_HANDLE;
		VulkanDebugHandler debugHandler;
		


  public:

	 VulkanInstance();

	 void vulkanInstanceCreator();


	VulkanInstance(const VulkanInstance&) = delete;

	~VulkanInstance();

	VkInstance& getInstance();

	VulkanDebugHandler&   getDebugHandler();

	const std::vector<const char*>& getValidationLayers() {
		return debugHandler.getValidationLayers();
	};


private:

    bool checkExtensions();

	void createVkInstance();

};
