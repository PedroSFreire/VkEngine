#include "../headers/Scene.h"
#include <iostream>
#include "../headers/GltfLoader.h"
#include <filesystem>


void Scene::loadFile(const std::string& filePath)
{
	GltfLoader gltfLoader;
	auto ext = std::filesystem::path(filePath).extension();
	if(ext == ".glm" || ext == ".gltf")
	{
		gltfLoader.loadGltf(scene,filePath.c_str());
		frameData.drawInstances.resize(scene.meshAssets.size());
	}
	else {
		std::cerr << "File was not a GLTF and GLTF loader used. " << std::endl;
	}

}

SceneFramesData& Scene::recordScene() {

	for (auto& drawInstance : frameData.drawInstances)
		drawInstance.cpuDrawCalls.clear();

	frameData.frameLightData.clear();

	glm::mat4 correction = glm::rotate(
		glm::mat4(1.0f),
		glm::radians(90.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	for (auto rootId : scene.rootNodesIds) {
		recordNode(rootId, correction);
	}
	return frameData;

}

void  Scene::recordNode(int nodeId, glm::mat4& transforMat) {

	glm::mat4 currentTransform = transforMat * scene.nodes[nodeId]->transform;

	//record mesh if exists
	if (scene.nodes[nodeId]->meshIndex.has_value()) {
		recordMesh(scene.nodes[nodeId]->meshIndex.value(), currentTransform);
	}

	//record light if exists
	if (scene.nodes[nodeId]->lightIndex.has_value()) {
		LightGPUData lightDataEntry;
		lightDataEntry.type = static_cast<uint32_t>(scene.lights[scene.nodes[nodeId]->lightIndex.value()]->type);
		lightDataEntry.color = scene.lights[scene.nodes[nodeId]->lightIndex.value()]->color;
		lightDataEntry.intensity = scene.lights[scene.nodes[nodeId]->lightIndex.value()]->intensity;
		lightDataEntry.range = scene.lights[scene.nodes[nodeId]->lightIndex.value()]->range;
		lightDataEntry.spotInnerCos = scene.lights[scene.nodes[nodeId]->lightIndex.value()]->spotInnerCos;
		lightDataEntry.spotOuterCos = scene.lights[scene.nodes[nodeId]->lightIndex.value()]->spotOuterCos;
		lightDataEntry.position = currentTransform * glm::vec4(0, 0, 0, 0);
		lightDataEntry.direction = currentTransform * glm::vec4(0, -1, 0, 0);

		frameData.frameLightData.emplace_back(std::move(lightDataEntry));
	}

	//call on children
	for (auto childId : scene.nodes[nodeId]->children) {
		recordNode( childId, currentTransform);
	}

}

void Scene::recordMesh(int meshId, glm::mat4& transform) {

	auto& meshAsset = scene.meshAssets[meshId];


	int prevSize = frameData.drawInstances[meshId].cpuDrawCalls.size();
	frameData.drawInstances[meshId].cpuDrawCalls.reserve(prevSize + meshAsset.get()->surfaces.size());
	frameData.drawInstances[meshId].meshId = meshId;
	frameData.drawInstances[meshId].meshResourceId = meshAsset->resourceId;
	for (int i = meshAsset.get()->surfaces.size() - 1; i >= 0; i--) {

		CPUDrawCallData newDrawCall;
		newDrawCall.materialDescriptorId = meshAsset->resourceId;
		newDrawCall.mat = (scene.materials[meshAsset.get()->surfaces[i].materialIndex]).get();
		newDrawCall.transform = transform;

		frameData.drawInstances[meshId].cpuDrawCalls.emplace_back(std::move(newDrawCall));

	}

}

