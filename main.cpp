#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION

#include "renderer\headers\VulkanRenderer.h"
#include "renderer\headers\Scene.h"
#include "renderer\headers\ResourceManager.h"



int main() {
	VulkanRenderer renderer;

	ResourceManager resourceManager(renderer);
	Scene scene;
	scene.loadFile("C:/Users/pedro/source/repos/VkEngine/scenes/ABeautifulGame/glTF/ABeautifulGame.gltf");
	resourceManager.loadScene(renderer,scene.getScene());

	try {
		while(renderer.running())
			renderer.run(scene,resourceManager);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}