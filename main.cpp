#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION

#include "renderer\headers\VulkanRenderer.h"




int main() {
	VulkanRenderer app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}