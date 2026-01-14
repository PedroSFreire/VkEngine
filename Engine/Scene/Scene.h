#pragma once
#include "../../Renderer/Platform/Window.h"
#include "../Core/defines.h"
#include "camera.h"

class VulkanRenderer;

class Scene
{
private:

	SceneFramesData frameData;

	std::vector<Camera> cameras;

	int activeCamera = 0;
	
	SceneData scene;

	void addDefaultLight();

public:

	Scene() = default;
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene(Scene&&) = default;


	Camera& getActiveCamera() { return cameras[activeCamera]; }
	
	bool lightsCreated = false;

	void loadFile(const std::string& filePath);
	SceneData& getScene() { return scene; }
	SceneFramesData& recordScene();

	void  recordNode( int nodeId, glm::mat4& transforMat);

	void recordMesh( int meshId, glm::mat4& transform);

	MeshAsset& getMeshAsset(uint32_t index) { return *scene.meshAssets[index]; }
};
