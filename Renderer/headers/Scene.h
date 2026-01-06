#pragma once
#include "../headers/defines.h"

class VulkanRenderer;

class Scene
{
private:

	SceneFramesData frameData;

	SceneData scene;

public:

	Scene() = default;
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene(Scene&&) = default;



	
	bool lightsCreated = false;

	void loadFile(const std::string& filePath);
	SceneData& getScene() { return scene; }
	SceneFramesData& recordScene();

	void  recordNode( int nodeId, glm::mat4& transforMat);

	void recordMesh( int meshId, glm::mat4& transform);

	MeshAsset& getMeshAsset(uint32_t index) { return *scene.meshAssets[index]; }
};
