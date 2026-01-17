#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION

#include "Renderer\Renderer\VulkanRenderer.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Resources\ResourceManager.h"

#include "Engine\Core\defines.h"


int main() {
	VulkanRenderer renderer;

	ResourceManager resourceManager(renderer);
	Scene scene;

	scene.loadFile("C:/Users/pedro/source/repos/VkEngine/scenes/ABeautifulGame/glTF/ABeautifulGame.gltf");
	//scene.loadFile("C:/Users/pedro/source/repos/VkEngine/scenes/Buggy/newBuggy.glb");
	//scene.loadFile("C:/Users/pedro/source/repos/VkEngine/scenes/test.glb");
	resourceManager.loadScene(renderer,scene.getScene());

	try {
		while (renderer.running()) {

			SceneFramesData& drawData = scene.recordScene();

			resourceManager.loadLights(renderer, drawData.frameLightData);

			renderer.run(drawData, resourceManager, scene.getActiveCamera());
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}