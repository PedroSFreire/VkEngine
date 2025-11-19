#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanDebugHandler.h" 
#include <iostream>


class VulkanInstance {


public:

	VulkanInstance();
	VulkanInstance(const VulkanInstance&) = delete;
	~VulkanInstance();


	void vulkanInstanceCreator();

	VkInstance getInstance() const { return instance; };

	const VulkanDebugHandler& getDebugHandler() const { return debugHandler; };

	const std::vector<const char*>& getValidationLayers() const { return debugHandler.getValidationLayers(); };


private:

	bool checkExtensions() const;

	void createVkInstance();


	VkInstance instance = VK_NULL_HANDLE;

	VulkanDebugHandler debugHandler;



	
};
