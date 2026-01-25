#include "Scene.h"
#include <iostream>
#include "../Loaders/GltfLoader.h"
#include <filesystem>
#include <glm/glm.hpp>


void Scene::addDefaultLight() {
	LightAsset newLight;
	newLight.type = LightType::Point;
	newLight.color.x = 0.8;
	newLight.color.y = 0.8;
	newLight.color.z = 0.8;
	newLight.intensity = 1000;
	newLight.range = 100.0f;
	scene.lights.emplace_back(std::make_shared<LightAsset>(std::move(newLight)));


	NodeAsset LightNode;

	LightNode.name = "defaultLight";

	LightNode.lightIndex = 0;

	LightNode.transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 1.0f, -5.0f));

	LightNode.children.emplace_back(scene.nodes.size()+1);

	scene.nodes.emplace_back(std::make_shared<NodeAsset>(std::move(LightNode)));

	NodeAsset meshNode;

	meshNode.name = "defaultMesh";

	meshNode.meshIndex = 0;

	meshNode.parentIndex = scene.nodes.size() - 1;

	meshNode.transform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f));

	scene.nodes.emplace_back(std::make_shared<NodeAsset>(std::move(meshNode)));

	scene.rootNodesIds.emplace_back(scene.nodes.size() - 2);

	//Add emissive mesh ball as child need to add material mesh emissive texture and node as child of light
}

void Scene::loadFile(const std::string& filePath)
{
	GltfLoader gltfLoader;
	Camera newCamera;
	cameras.resize(1);
	cameras[0] = std::move(newCamera);
	auto ext = std::filesystem::path(filePath).extension();
	if(ext == ".glm" || ext == ".gltf" || ext == ".glb")
	{
		gltfLoader.loadGltf(scene,filePath.c_str());
		if(scene.lights.size() == 0)
			addDefaultLight();
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

	//glm::mat4 correction = glm::rotate(glm::mat4(1.0f),glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 correction = glm::mat4(1.0f);
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
		lightDataEntry.position = currentTransform * glm::vec4(0, 0, 0, 1);

		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(currentTransform)));
		lightDataEntry.direction = normalMat * glm::vec3(0, 0, -1);

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
	if (meshAsset->resourceId == -1)
		printf("");
	for (int i = 0; i < meshAsset.get()->surfaces.size(); i++) {

		CPUDrawCallData newDrawCall;
		newDrawCall.startIndex = meshAsset->surfaces[i].startIndex;
		newDrawCall.count = meshAsset->surfaces[i].count;
		newDrawCall.materialDescriptorId = meshAsset->resourceId;
		newDrawCall.mat = (scene.materials[meshAsset.get()->surfaces[i].materialIndex]).get();
		newDrawCall.transform = transform;

		frameData.drawInstances[meshId].cpuDrawCalls.emplace_back(std::move(newDrawCall));

	}

}

