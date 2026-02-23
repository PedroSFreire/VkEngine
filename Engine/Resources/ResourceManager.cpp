#include "ResourceManager.h"

#include "../../Renderer/Renderer/DescriptorManager.h"
#include "../../Renderer/Renderer/VulkanRenderer.h"
#include "stb_image.h"







static constexpr VkFormat TextureTypeToVkFormat(TextureType type) {
	switch (type) {
	case TextureType::Color:
		return VK_FORMAT_R8G8B8A8_SRGB;

	case TextureType::Normal:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case TextureType::MetallicRoughnessAO:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case TextureType::Emissive:
		return VK_FORMAT_R8G8B8A8_SRGB;

	case TextureType::Height:
		return VK_FORMAT_R8_UNORM;

	case TextureType::Occlusion:
		return VK_FORMAT_R8_UNORM;
	case TextureType::HDRColor:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	return VK_FORMAT_UNDEFINED;
}


static constexpr VkFilter toVkFilter(Filter f) {
	switch (f) {
		case Filter::Nearest: return VK_FILTER_NEAREST;
		case Filter::Linear:  return VK_FILTER_LINEAR;
	}
	return VK_FILTER_LINEAR; 
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
	createSampler(renderer,defaultSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	createSampler(renderer, defaultCubeSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	createSimpleMeshResources(renderer, cubeVertices, cubeIndices, cubeMesh);
	createEnvironmentMaps(renderer, "textures/monks_forest_4k.hdr");
}

uint32_t ResourceManager::loadImage(const VulkanRenderer& renderer, const ImageAsset& img) {
	ImageResource newImage;
	newImage.name = img.name;
	createTexture(renderer,img, newImage );
	images.emplace_back(std::make_shared<ImageResource>(std::move(newImage)));
	return static_cast<uint32_t>(images.size() - 1);
}

uint32_t ResourceManager::loadSampler(const VulkanRenderer& renderer, const SamplerAsset& sampler) {
	SamplerResource newSampler;



	createSampler(renderer, newSampler, toVkFilter(sampler.magFilter), toVkFilter(sampler.minFilter),
		toVkMipmapMode(sampler.mipMap),
		toVkAddressMode(sampler.addressU), toVkAddressMode(sampler.addressV));
	samplers.emplace_back(std::make_shared<SamplerResource>(std::move(newSampler)));
	return static_cast<uint32_t>(samplers.size() - 1);
}

uint32_t ResourceManager::loadMesh(const VulkanRenderer& renderer, const MeshAsset& mesh) {
	MeshResource newMesh;
	newMesh.name = mesh.name;
	createMeshResources(renderer,mesh.vertices, mesh.indices, newMesh.meshBuffers);
	meshes.emplace_back(std::make_shared<MeshResource>(std::move(newMesh)));
	return static_cast<uint32_t>(meshes.size() - 1);
}

uint32_t ResourceManager::createDescriptorSet(const VulkanRenderer& renderer, const MaterialAsset& mat, SceneData& scene) {
	VulkanDescriptorSet newSet;
	DescriptorManager::createMaterialDescriptorLayout(renderer.getLogicalDevice(),newSet, descriptorPool);
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
	
	DescriptorManager::updateMaterialDescriptor(newSet, imageViews.data(), imageSamplers.data());
	descriptorSets.emplace_back(std::move(newSet));
	return static_cast<uint32_t>(descriptorSets.size() - 1);

}


void ResourceManager::createDefaultImages(const VulkanRenderer& renderer) {

	// --- Default Color Texture (SRGB) ---
	std::array<uint8_t, 4> colorPixel{ 255, 255, 255, 255 }; // white
	defaultColorImage.name = "color";
	ImageAsset colorData;
	colorData.width = 1;
	colorData.height = 1;
	colorData.channels = 4;
	colorData.type = TextureType::Color;
	colorData.pixels = colorPixel.data();
	createTexture(renderer, colorData, defaultColorImage);

	// --- Default Normal Texture (UNORM) ---
	std::array<uint8_t, 4> normalPixel{ 128, 128, 255, 255 }; // neutral normal map
	defaultNormalImage.name = "normal";
	ImageAsset normalData;
	normalData.width = 1;
	normalData.height = 1;
	normalData.channels = 4;
	normalData.type = TextureType::Normal;
	normalData.pixels = normalPixel.data();
	createTexture(renderer, normalData, defaultNormalImage);

	// --- Default Metallic-Roughness-AO Texture (UNORM) ---
	std::array<uint8_t, 4> metalRoughPixel{ 0, 255, 255, 255 }; // R=metal, G=rough, B=AO
	defaultMetalRoughImage.name = "metalRough";
	ImageAsset metalRoughData;
	metalRoughData.width = 1;
	metalRoughData.height = 1;
	metalRoughData.channels = 4;
	metalRoughData.type = TextureType::MetallicRoughnessAO;
	metalRoughData.pixels = metalRoughPixel.data();
	createTexture(renderer, metalRoughData, defaultMetalRoughImage);

	// --- Default Occlusion Texture (R8_UNORM) ---
	std::array<uint8_t, 4> occlusionPixel{ 255, 255, 255, 255 }; // fully unoccluded
	defaultOcclusionImage.name = "occlusion";
	ImageAsset occlusionData;
	occlusionData.width = 1;
	occlusionData.height = 1;
	occlusionData.channels = 4;
	occlusionData.type = TextureType::Occlusion;
	occlusionData.pixels = occlusionPixel.data();
	createTexture(renderer, occlusionData, defaultOcclusionImage);

	// --- Default Emissive Texture (SRGB) ---
	std::array<uint8_t, 4> emissivePixel{ 0, 0, 0, 255 }; // black (no emission)
	defaultEmissiveImage.name = "emissive";
	ImageAsset emissiveData;
	emissiveData.width = 1;
	emissiveData.height = 1;
	emissiveData.channels = 4;
	emissiveData.type = TextureType::Emissive;
	emissiveData.pixels = emissivePixel.data();
	createTexture(renderer, emissiveData, defaultEmissiveImage);
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
		bufferStagedUpload(renderer,lightBuffer, lights.data(), static_cast<uint32_t>(sizeof(LightGPUData) * lights.size()), 1);
		DescriptorManager::updateLightDescriptor(lightDescriptorSet,lightBuffer, lights.size());
	
	
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

	DescriptorManager::createMaterialDescriptorPool(renderer.getLogicalDevice(),descriptorPool, scene.materials.size());
	descriptorSets.reserve(scene.materials.size());
	for (auto& mat : scene.materials) {
		mat->resourceId = createDescriptorSet(renderer, *(mat.get()), scene);
	}


	DescriptorManager::createLightDescriptorPool(renderer.getLogicalDevice(), lightdescriptorPool,1);
	DescriptorManager::createLightDescriptorLayout(renderer.getLogicalDevice(), lightDescriptorSet, lightdescriptorPool);
	lightDescriptorSet.createDescriptor();
}



void ResourceManager::copyBuffer(const VulkanRenderer& renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const {
	VulkanCommandBuffer commandBuffer;
	commandBuffer.beginRecordindSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

	commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());

}

void ResourceManager::createMeshResources(const VulkanRenderer& renderer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const {

	createVertexBuffer(renderer, vertices, meshBuffer.vertexBuffer);
	createIndexBuffer(renderer, indices, meshBuffer.indexBuffer);

}

void ResourceManager::createVertexBuffer(const VulkanRenderer& renderer, const std::vector<Vertex>& vertices, VulkanBuffer& vertexBuffer) const {

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	uint32_t vextexCount = static_cast<uint32_t>(vertices.size());

	//create buffer
	VulkanBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.elementCount = vextexCount;
	vertexBufferInfo.size = bufferSize;
	vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	vertexBuffer.createBuffer(renderer.getAllocator(), vertexBufferInfo);

	//upload data
	bufferStagedUpload(renderer, vertexBuffer, vertices.data(), bufferSize, vextexCount);

}

void ResourceManager::createSimpleMeshResources(const VulkanRenderer& renderer, const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const {

	createSimpleVertexBuffer(renderer, vertices, meshBuffer.vertexBuffer);
	createIndexBuffer(renderer, indices, meshBuffer.indexBuffer);

}

void ResourceManager::createSimpleVertexBuffer(const VulkanRenderer& renderer, const std::vector<SimpleVertex>& vertices, VulkanBuffer& vertexBuffer) const {

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	uint32_t vextexCount = static_cast<uint32_t>(vertices.size());

	//create buffer
	VulkanBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.elementCount = vextexCount;
	vertexBufferInfo.size = bufferSize;
	vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	vertexBuffer.createBuffer(renderer.getAllocator(), vertexBufferInfo);

	//upload data
	bufferStagedUpload(renderer, vertexBuffer, vertices.data(), bufferSize, vextexCount);

}

void ResourceManager::createIndexBuffer(const VulkanRenderer& renderer, const std::vector<uint32_t>& indices, VulkanBuffer& indexBuffer) const {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	uint32_t indexCount = static_cast<uint32_t>(indices.size());
	VulkanBuffer stagingBuffer;

	//create buffer
	VulkanBufferCreateInfo indexBufferInfo{};
	indexBufferInfo.elementCount = indexCount;
	indexBufferInfo.size = bufferSize;
	indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	indexBuffer.createBuffer(renderer.getAllocator(), indexBufferInfo);

	//upload data
	bufferStagedUpload(renderer,indexBuffer, indices.data(), bufferSize, indexCount);
}

void ResourceManager::bufferStagedUpload(const VulkanRenderer& renderer, VulkanBuffer& dstBuffer, const void* bufferData, uint32_t size, uint32_t elementCount) const {

	VkDeviceSize bufferSize = size;


	VulkanBuffer stagingBuffer;

	VulkanBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.elementCount = elementCount;
	stagingBufferInfo.size = bufferSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.vmaUsage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferInfo.vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	stagingBuffer.createBuffer(renderer.getAllocator(), stagingBufferInfo);


	void* data;
	vmaMapMemory(renderer.getAllocator().getAllocator(), stagingBuffer.getAllocation(), &data);
	memcpy(data, bufferData, (size_t)bufferSize);
	vmaUnmapMemory(renderer.getAllocator().getAllocator(), stagingBuffer.getAllocation());


	copyBuffer(renderer,stagingBuffer.getBuffer(), dstBuffer.getBuffer(), bufferSize);
}


void ResourceManager::createTexture(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, ImageResource& tex, TextureType type) const {
	createTextureImage(renderer,TEXTURE_PATH, tex.image ,type);
	tex.imageView.createImageView(renderer.getLogicalDevice(), tex.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void ResourceManager::createTexture(const VulkanRenderer& renderer, const ImageAsset& data, ImageResource& tex) const {
	createTextureImage(renderer, data, tex.image);
	tex.imageView.createImageView(renderer.getLogicalDevice(), tex.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void ResourceManager::createTextureImage(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, VulkanImage& textureImage, TextureType type) const {
	int texWidth, texHeight, texChannels;


	if (static_cast<int>(type) & static_cast<int>(TextureType::HDRColor)) {
		//stbi_set_flip_vertically_on_load(true);
		float* pixels = stbi_loadf(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, 0);
		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		std::vector<float> rgba(texWidth * texHeight * 4);

		for (int i = 0; i < texWidth * texHeight; i++) {
			rgba[i * 4 + 0] = pixels[i * 3 + 0];
			rgba[i * 4 + 1] = pixels[i * 3 + 1];
			rgba[i * 4 + 2] = pixels[i * 3 + 2];
			rgba[i * 4 + 3] = 1.0f;
		}
		createTextureImageHelper(renderer, rgba.data(), texWidth, texHeight, textureImage, type);
		stbi_image_free(pixels);
		stbi_set_flip_vertically_on_load(false);
	}
	else {
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		createTextureImageHelper(renderer, pixels, texWidth, texHeight, textureImage, type);
		stbi_image_free(pixels);
	}
}

void ResourceManager::createTextureImage(const VulkanRenderer& renderer, const ImageAsset& data, VulkanImage& textureImage)const {

	VkDeviceSize imageSize = data.width * data.height * 4;
	createTextureImageHelper(renderer, data.pixels, data.width, data.height, textureImage,data.type);


}

//should be template?
void ResourceManager::createTextureImageHelper(const VulkanRenderer& renderer, stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage, TextureType type) const {

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VulkanBuffer stagingBuffer;

	VulkanBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.elementCount = 1;
	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.vmaUsage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferInfo.vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;


	stagingBuffer.createBuffer(renderer.getAllocator(), stagingBufferInfo);

	void* data;

	vmaMapMemory(renderer.getAllocator().getAllocator(), stagingBuffer.getAllocation(), &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vmaUnmapMemory(renderer.getAllocator().getAllocator(), stagingBuffer.getAllocation());

	VulkanImageCreateInfo textureImageInfo{};
	textureImageInfo.width = static_cast<uint32_t>(texWidth);
	textureImageInfo.height = static_cast<uint32_t>(texHeight);
	textureImageInfo.format = TextureTypeToVkFormat(type);
	textureImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;


	textureImage.create2DImage(renderer.getAllocator(), textureImageInfo);

	transitionImageLayout(renderer, textureImage, TextureTypeToVkFormat(type), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,1);

	copyBufferToImage(renderer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(renderer, textureImage, TextureTypeToVkFormat(type), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, renderer.getPhysicalDevice().findQueueFamilies(renderer.getSurface()).transferFamily.value()
		, renderer.getPhysicalDevice().findQueueFamilies(renderer.getSurface()).graphicsFamily.value(),1);

	transitionImageLayout(renderer, textureImage, TextureTypeToVkFormat(type), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,1);
}

void ResourceManager::createTextureImageHelper(const VulkanRenderer& renderer, float* pixels, int texWidth, int texHeight, VulkanImage& textureImage, TextureType type) const {

	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);
	VulkanBuffer stagingBuffer;

	VulkanBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.elementCount = 1;
	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.vmaUsage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferInfo.vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;


	stagingBuffer.createBuffer(renderer.getAllocator(), stagingBufferInfo);

	void* data;

	vmaMapMemory(renderer.getAllocator().getAllocator(), stagingBuffer.getAllocation(), &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vmaUnmapMemory(renderer.getAllocator().getAllocator(), stagingBuffer.getAllocation());

	VulkanImageCreateInfo textureImageInfo{};
	textureImageInfo.width = static_cast<uint32_t>(texWidth);
	textureImageInfo.height = static_cast<uint32_t>(texHeight);
	textureImageInfo.format = TextureTypeToVkFormat(type);
	textureImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;


	textureImage.create2DImage(renderer.getAllocator(), textureImageInfo);

	transitionImageLayout(renderer, textureImage, TextureTypeToVkFormat(type), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,1);

	copyBufferToImage(renderer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(renderer, textureImage, TextureTypeToVkFormat(type), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, renderer.getPhysicalDevice().findQueueFamilies(renderer.getSurface()).transferFamily.value()
		, renderer.getPhysicalDevice().findQueueFamilies(renderer.getSurface()).graphicsFamily.value(),1);

	transitionImageLayout(renderer, textureImage, TextureTypeToVkFormat(type), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,1);
}

void ResourceManager::createSampler(const VulkanRenderer& renderer, SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV) const {


	samplerResource.sampler.createTextureSampler(renderer.getPhysicalDevice(), renderer.getLogicalDevice(), magFilter, minFilter, mipMap, addressU, adressV, VK_SAMPLER_ADDRESS_MODE_REPEAT);

}

void ResourceManager::createSampler(const VulkanRenderer& renderer, SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV, VkSamplerAddressMode adressW) const {


	samplerResource.sampler.createTextureSampler(renderer.getPhysicalDevice(), renderer.getLogicalDevice(), magFilter, minFilter, mipMap, addressU, adressV, adressW);

}


void ResourceManager::transitionImageLayout(const VulkanRenderer& renderer, VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue, uint32_t layerCount , uint32_t levelCount)const {

	VulkanCommandBuffer commandBuffer;
	if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) || (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) || (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED  && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
		commandBuffer.beginRecordindSingleTimeCommands(renderer.getLogicalDevice(), renderer.getCommandPool());
	}
	else {
		commandBuffer.beginRecordindSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());
	}


	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;


	barrier.srcQueueFamilyIndex = srcQueue;
	barrier.dstQueueFamilyIndex = destQueue;

	barrier.image = image.getImage();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = levelCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		vkCmdPipelineBarrier(
			commandBuffer.getCommandBuffer(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		vkCmdPipelineBarrier(
			commandBuffer.getCommandBuffer(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		vkCmdPipelineBarrier(
			commandBuffer.getCommandBuffer(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getCommandPool());
	}
	else if (oldLayout ==  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout ==  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		vkCmdPipelineBarrier(
			commandBuffer.getCommandBuffer(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// End the single-time command buffer so the layout transition executes
		commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getCommandPool());
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		vkCmdPipelineBarrier(
			commandBuffer.getCommandBuffer(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getCommandPool());
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}



}

void ResourceManager::copyBufferToImage(const VulkanRenderer& renderer, VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height)const {
	VulkanCommandBuffer commandBuffer;
	commandBuffer.beginRecordindSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer.getCommandBuffer(),
		buffer.getBuffer(),
		image.getImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	commandBuffer.endRecordingSingleTimeCommands(renderer.getLogicalDevice(), renderer.getTransferCommandPool());
}


//env maps
void ResourceManager::createCubeImage(const VulkanRenderer& renderer, CubeMapResource& resource , uint32_t textSize, uint32_t mipLevels) {
	//to be implemented
	VulkanImageCreateInfo textureImageInfo{};
	textureImageInfo.width = textSize;
	textureImageInfo.height = textSize;
	textureImageInfo.format = TextureTypeToVkFormat(TextureType::HDRColor);
	textureImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
	textureImageInfo.layers = 6;
	textureImageInfo.mipLevels = mipLevels;
	textureImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	resource.image.create2DImage(renderer.getAllocator(), textureImageInfo);
	for (int layer = 0; layer < 6; ++layer)
	{
		resource.imageViews[layer].createImageView(renderer.getLogicalDevice(), resource.image, layer, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	resource.name = "environment_cubemap";
	resource.cubeImageView.createCubeImageView(renderer.getLogicalDevice(), resource.image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}


void ResourceManager::createEnvironmentMaps(const VulkanRenderer& renderer, const std::string& HDRI_PATH) {
	

	//create cube descriptors
	DescriptorManager::createCubeDescriptorPool(renderer.getLogicalDevice(), cubeDescriptorPool, 3);
	DescriptorManager::createCubeDescriptorLayout(renderer.getLogicalDevice(), cubeDescriptorSet, cubeDescriptorPool);
	DescriptorManager::createCubeDescriptorLayout(renderer.getLogicalDevice(), irradianceCubeDescriptorSet, cubeDescriptorPool);
	DescriptorManager::createCubeDescriptorLayout(renderer.getLogicalDevice(), prefilteredCubeDescriptorSet, cubeDescriptorPool);
	cubeDescriptorSet.createDescriptor();
	irradianceCubeDescriptorSet.createDescriptor();
	prefilteredCubeDescriptorSet.createDescriptor();

	uint32_t textSize = 256;
	// allocate cubemap image
	createCubeImage(renderer, cubemapImage, textSize*4);
	createCubeImage(renderer, irradianceImage, textSize);
	createCubeImage(renderer, prefilteredImage, textSize, static_cast<uint32_t>(std::floor(std::log2(textSize))) + 1);

	// sampler for equi
	VulkanSampler envSampler;
	envSampler.createTextureSampler(renderer.getPhysicalDevice(), renderer.getLogicalDevice(), VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
	//load equirectangular HDR image

	createTexture(renderer, HDRI_PATH, equirectangularImage, TextureType::HDRColor);
	

	//calculate cube map from equirectangular map
	renderer.cubePass(equirectangularImage.imageView, cubemapImage, envSampler, cubeMesh,textSize*4,false/*not irradiance pass*/);

	transitionImageLayout(renderer, cubemapImage.image, TextureTypeToVkFormat(TextureType::HDRColor), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,6);
	DescriptorManager::updateCubeDescriptor(cubeDescriptorSet, cubemapImage.cubeImageView.getImageView(),defaultCubeSampler.sampler.getSampler());

	//calculate irradiance map from cubemap
	renderer.cubePass(cubemapImage.cubeImageView, irradianceImage, defaultCubeSampler.sampler, cubeMesh, textSize, true/*irradiance pass*/);

	transitionImageLayout(renderer, irradianceImage.image, TextureTypeToVkFormat(TextureType::HDRColor), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, 6);
	DescriptorManager::updateCubeDescriptor(irradianceCubeDescriptorSet, irradianceImage.cubeImageView.getImageView(), defaultCubeSampler.sampler.getSampler());

	//calculate prefiltered map from cubemap
	uint32_t prefilteredLayerCount = std::floor(std::log2(textSize)) + 1;
	renderer.prefilteredCubePass(cubemapImage.cubeImageView, prefilteredImage, defaultCubeSampler.sampler, cubeMesh, textSize);


	transitionImageLayout(renderer, prefilteredImage.image, TextureTypeToVkFormat(TextureType::HDRColor), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, 6, prefilteredLayerCount);
	DescriptorManager::updateCubeDescriptor(prefilteredCubeDescriptorSet, prefilteredImage.cubeImageView.getImageView(), defaultCubeSampler.sampler.getSampler());

}