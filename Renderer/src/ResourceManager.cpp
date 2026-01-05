#include "../headers/ResourceManager.h"
#include "../headers/VulkanRenderer.h"
#include "../headers/defines.h"
#include "../headers/Scene.h"


static constexpr VkFilter toVkFilter(Filter f) {
	switch (f) {
		case Filter::Nearest: return VK_FILTER_NEAREST;
		case Filter::Linear:  return VK_FILTER_LINEAR;
	}
	return VK_FILTER_LINEAR; // fallback
}


static constexpr VkSamplerMipmapMode toVkMipmapMode(MipmapMode m) {
	switch (m) {
	case MipmapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case MipmapMode::Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
	return VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

static constexpr VkSamplerAddressMode toVkAddressMode(AddressMode a) {
	switch (a) {
	case AddressMode::Repeat:          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case AddressMode::MirroredRepeat:  return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case AddressMode::ClampToEdge:     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case AddressMode::ClampToBorder:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}
	return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}


ResourceManager::ResourceManager(const VulkanRenderer& renderer) {
	createDefaultImages(renderer);
	renderer.createSampler(defaultSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

}

uint32_t ResourceManager::loadImage(const VulkanRenderer& renderer, const ImageAsset& img) {
	ImageResource newImage;
	newImage.name = img.name;
	renderer.createTexture(img, newImage);
	images.emplace_back(std::make_shared<ImageResource>(std::move(newImage)));
	return static_cast<uint32_t>(images.size() - 1);
}

uint32_t ResourceManager::loadSampler(const VulkanRenderer& renderer, const SamplerAsset& sampler) {
	SamplerResource newSampler;



	renderer.createSampler(newSampler, toVkFilter(sampler.magFilter), toVkFilter(sampler.minFilter),
		toVkMipmapMode(sampler.mipMap),
		toVkAddressMode(sampler.addressU), toVkAddressMode(sampler.addressV));
	samplers.emplace_back(std::make_shared<SamplerResource>(std::move(newSampler)));
	return static_cast<uint32_t>(samplers.size() - 1);
}

uint32_t ResourceManager::loadMesh(const VulkanRenderer& renderer, const MeshAsset& mesh) {
	MeshResource newMesh;
	newMesh.name = mesh.name;
	renderer.createMeshResources(mesh.vertices, mesh.indices, newMesh.meshBuffers);
	meshes.emplace_back(std::make_shared<MeshResource>(std::move(newMesh)));
	return static_cast<uint32_t>(meshes.size() - 1);
}

uint32_t ResourceManager::createDescriptorSet(const VulkanRenderer& renderer, const MaterialAsset& mat, SceneData& scene) {
	VulkanDescriptorSet newSet;
	newSet.createMaterialDescriptorLayout(renderer.getLogicalDevice(), descriptorPool);
	newSet.createDescriptor();
	//TODO:load default textures
	ImageResource* colorImage;
	SamplerResource* colorSampler;

	ImageResource* normalImage;
	SamplerResource* normalSampler;

	ImageResource* metalRoughImage;
	SamplerResource* metalRoughSampler;

	ImageResource* occlusionImage;
	SamplerResource* occlusionSampler;

	ImageResource* emissiveImage;
	SamplerResource* emissiveSampler;

	//load color texture
	if (mat.colorTexId.has_value()) {
		auto& colorTexture = scene.textures[mat.colorTexId.value()];
		colorImage = images[scene.imageAssets[colorTexture->imageId]->resourceId].get();
		colorSampler = samplers[scene.samplerAssets[colorTexture->samplerId]->resourceId].get();

	}
	else {
		colorImage = &defaultColorImage;
		colorSampler = &defaultSampler;
	}

	//load nomral texture
	if (mat.normalTexId.has_value()) {
		auto& normalTexture = scene.textures[mat.normalTexId.value()];
		normalImage = images[scene.imageAssets[normalTexture->imageId]->resourceId].get();
		normalSampler = samplers[scene.samplerAssets[normalTexture->samplerId]->resourceId].get();

	}
	else {
		normalImage = &defaultNormalImage;
		normalSampler = &defaultSampler;
	}

	//load metalRough texture
	if (mat.metalRoughTexId.has_value()) {
		auto& metalRoughTexture = scene.textures[mat.metalRoughTexId.value()];
		metalRoughImage = images[scene.imageAssets[metalRoughTexture->imageId]->resourceId].get();
		metalRoughSampler = samplers[scene.samplerAssets[metalRoughTexture->samplerId]->resourceId].get();

	}
	else {
		metalRoughImage = &defaultMetalRoughImage;
		metalRoughSampler = &defaultSampler;
	}

	//load occlusion texture
	if (mat.occlusionTexId.has_value()) {
		auto& occlusionTexture = scene.textures[mat.occlusionTexId.value()];
		occlusionImage = images[scene.imageAssets[occlusionTexture->imageId]->resourceId].get();
		occlusionSampler = samplers[scene.samplerAssets[occlusionTexture->samplerId]->resourceId].get();

	}
	else {
		occlusionImage = &defaultOcclusionImage;
		occlusionSampler = &defaultSampler;
	}

	//load emissive texture
	if (mat.emissiveTexId.has_value()) {
		auto& emissiveTexture = scene.textures[mat.emissiveTexId.value()];
		emissiveImage = images[scene.imageAssets[emissiveTexture->imageId]->resourceId].get();
		emissiveSampler = samplers[scene.samplerAssets[emissiveTexture->samplerId]->resourceId].get();

	}
	else {
		emissiveImage = &defaultEmissiveImage;
		emissiveSampler = &defaultSampler;
	}

	std::array<VkImageView, 5> imageViews{ colorImage->imageView.getImageView(),normalImage->imageView.getImageView(),metalRoughImage->imageView.getImageView(),occlusionImage->imageView.getImageView(),emissiveImage->imageView.getImageView() };
	std::array<VkSampler, 5> imageSamplers{ colorSampler->sampler.getSampler(),normalSampler->sampler.getSampler(),metalRoughSampler->sampler.getSampler(),occlusionSampler->sampler.getSampler(),emissiveSampler->sampler.getSampler() };
	newSet.updateMaterialDescriptor(imageViews.data(), imageSamplers.data());
	descriptorSets.emplace_back(std::move(newSet));
	return static_cast<uint32_t>(descriptorSets.size() - 1);

}


void ResourceManager::createDefaultImages(const VulkanRenderer& renderer) {


	//create color default tex
	std::array<uint8_t, 4> colorPixel{ 255, 255, 255, 255 };
	defaultColorImage.name = "color";
	ImageAsset imgData;
	imgData.height = 1;
	imgData.width = 1;
	imgData.channels = 4;
	imgData.pixels = colorPixel.data();
	renderer.createTexture(imgData, defaultColorImage);

	//create normal default tex
	std::array<uint8_t, 4> normalPixel{ 255, 255, 255, 255 };
	defaultNormalImage.name = "normal";
	ImageAsset normalData;
	normalData.height = 1;
	normalData.width = 1;
	normalData.channels = 4;
	normalData.pixels = normalPixel.data();
	renderer.createTexture(normalData, defaultNormalImage);

	//create normal metal rough tex
	std::array<uint8_t, 4> metalRoughPixel{ 255, 255, 255, 255 };
	defaultMetalRoughImage.name = "metalRough";
	ImageAsset metalRoughData;
	metalRoughData.height = 1;
	metalRoughData.width = 1;
	metalRoughData.channels = 4;
	metalRoughData.pixels = metalRoughPixel.data();
	renderer.createTexture(metalRoughData, defaultMetalRoughImage);

	//create occlusion default tex
	std::array<uint8_t, 4> occlusionPixel{ 255, 255, 255, 255 };
	defaultOcclusionImage.name = "occlusion";
	ImageAsset occlusionData;
	occlusionData.height = 1;
	occlusionData.width = 1;
	occlusionData.channels = 4;
	occlusionData.pixels = occlusionPixel.data();
	renderer.createTexture(occlusionData, defaultOcclusionImage);

	//create emissive default tex
	std::array<uint8_t, 4> emissivePixel{ 255, 255, 255, 255 };
	defaultEmissiveImage.name = "emissive";
	ImageAsset emissiveData;
	emissiveData.height = 1;
	emissiveData.width = 1;
	emissiveData.channels = 4;
	emissiveData.pixels = emissivePixel.data();
	renderer.createTexture(emissiveData, defaultEmissiveImage);

}

void ResourceManager::loadLights(const VulkanRenderer& renderer, std::vector<LightGPUData>& lights) {
	if (!lightsCreated) {
		VulkanBufferCreateInfo info;
		info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		info.size = sizeof(LightGPUData) * lights.size();
		info.elementCount = static_cast<uint32_t>(lights.size());
		info.vmaFlags = VMA_MEMORY_USAGE_GPU_ONLY;


		lightBuffer.createBuffer(renderer.getAllocator(), info);
		lightsCreated = true;
	}
		renderer.bufferStagedUpload(lightBuffer, lights.data(), static_cast<uint32_t>(sizeof(LightGPUData) * lights.size()), 1);
		lightDescriptorSet.updateLightDescriptor(lightBuffer, lights.size());
	
	
}


void ResourceManager::loadScene(const VulkanRenderer& renderer,SceneData& scene) {
	
	images.reserve(images.size() + scene.imageAssets.size());
	for (auto& image : scene.imageAssets) {
		image->resourceId = loadImage(renderer, *(image.get()));
	}

	meshes.reserve(meshes.size() + scene.meshAssets.size());
	for (auto& mesh : scene.meshAssets) {
		mesh->resourceId = loadMesh(renderer, *(mesh.get()));
	}

	samplers.reserve(samplers.size() + scene.samplerAssets.size());
	for (auto& sampler : scene.samplerAssets) {
		sampler->resourceId = loadSampler(renderer, *(sampler.get()));
	}


	//descriptor pool needs scaling but for that new pools would be needed TODO
	descriptorPool.createMaterialDescriptorPool(renderer.getLogicalDevice(), scene.imageAssets.size());
	descriptorSets.reserve(descriptorSets.size() +scene.imageAssets.size());
	for (auto& mat : scene.materials) {
		mat->resourceId = createDescriptorSet(renderer, *(mat.get()), scene);
	}

	lightdescriptorPool.createLightDescriptorPool(renderer.getLogicalDevice(), 1);
	lightDescriptorSet.createLightDescriptorLayout(renderer.getLogicalDevice(), lightdescriptorPool);
	lightDescriptorSet.createDescriptor();
}