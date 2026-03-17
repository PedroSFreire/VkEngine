#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Renderer\Renderer\VulkanRenderer.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Resources\ResourceManager.h"
#include "Engine\UI\ImGuiManager.h"

#include "Engine\Core\defines.h"


#include <chrono>

std::chrono::high_resolution_clock timer;
std::chrono::time_point<std::chrono::high_resolution_clock> prevTime,currentTime;
float deltaTime;



int main() {
	VulkanRenderer renderer;

	ImGuiManager imGuiManager(renderer);

	ResourceManager resourceManager(renderer);
	Scene scene;

	std::string pathToRoot = "../../../";
	//scene.loadFile(pathToRoot + "scenes/BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf");
	//scene.loadFile(pathToRoot + "scenes/Buggy/newBuggy.glb");
	//scene.loadFile(pathToRoot + "scenes/testWlights.glb");
	//scene.loadFile(pathToRoot + "scenes/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf");
	//scene.loadFile(pathToRoot + "scenes/LightsPunctualLamp/glTF-Binary/LightsPunctualLamp.glb");
	//scene.loadFile(pathToRoot + "scenes/SpecularTest/glTF/SpecularTest.gltf");
	//scene.loadFile(pathToRoot + "scenes/DamagedHelmet/glTF/DamagedHelmet.gltf");
	//scene.loadFile(pathToRoot + "scenes/sponzaLight.glb");
	//scene.loadFile(pathToRoot + "scenes/bb.glb");
	//resourceManager.loadScene(renderer,scene.getScene());
	prevTime = timer.now();

	try {
		while (renderer.running()) {

			if (imGuiManager.isSelected() && !scene.isLoaded()) {
				scene.loadFile(imGuiManager.getSelectedPath());
				resourceManager.loadScene(renderer, scene.getScene());
			}

			currentTime = timer.now();

			deltaTime = std::chrono::duration<float, std::milli>(currentTime - prevTime).count();

			prevTime = currentTime;

			
			if (!imGuiManager.isSelected()) {
				imGuiManager.startUpFrame();
			}else {
				imGuiManager.recordFrame(deltaTime);
			}


			SceneFramesData drawData;

			if (scene.isLoaded()) {
				drawData = scene.recordScene();
				resourceManager.loadLights(renderer, drawData.frameLightData);
			}


			renderer.run(drawData, resourceManager, scene.getActiveCamera(), imGuiManager);
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}