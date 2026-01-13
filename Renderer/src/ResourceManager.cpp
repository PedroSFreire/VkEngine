#include "../headers/ResourceManager.h"
#include "../headers/VulkanRenderer.h"
#include "../headers/DescriptorManager.h"


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
	createSampler(renderer,defaultSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

}

uint32_t ResourceManager::loadImage(const VulkanRenderer& renderer, const ImageAsset& img) {
	ImageResource newImage;
	newImage.name = img.name;
	createTexture(renderer,img, newImage);
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


	//create color default tex
	std::array<uint8_t, 4> colorPixel{ 255, 255, 255, 255 };
	defaultColorImage.name = "color";
	ImageAsset imgData;
	imgData.height = 1;
	imgData.width = 1;
	imgData.channels = 4;
	imgData.pixels = colorPixel.data();
	createTexture(renderer,imgData, defaultColorImage);

	//create normal default tex
	std::array<uint8_t, 4> normalPixel{ 255, 255, 255, 255 };
	defaultNormalImage.name = "normal";
	ImageAsset normalData;
	normalData.height = 1;
	normalData.width = 1;
	normalData.channels = 4;
	normalData.pixels = normalPixel.data();
	createTexture(renderer,normalData, defaultNormalImage);

	//create normal metal rough tex
	std::array<uint8_t, 4> metalRoughPixel{ 255, 255, 255, 255 };
	defaultMetalRoughImage.name = "metalRough";
	ImageAsset metalRoughData;
	metalRoughData.height = 1;
	metalRoughData.width = 1;
	metalRoughData.channels = 4;
	metalRoughData.pixels = metalRoughPixel.data();
	createTexture(renderer,metalRoughData, defaultMetalRoughImage);

	//create occlusion default tex
	std::array<uint8_t, 4> occlusionPixel{ 255, 255, 255, 255 };
	defaultOcclusionImage.name = "occlusion";
	ImageAsset occlusionData;
	occlusionData.height = 1;
	occlusionData.width = 1;
	occlusionData.channels = 4;
	occlusionData.pixels = occlusionPixel.data();
	createTexture(renderer,occlusionData, defaultOcclusionImage);

	//create emissive default tex
	std::array<uint8_t, 4> emissivePixel{ 255, 255, 255, 255 };
	defaultEmissiveImage.name = "emissive";
	ImageAsset emissiveData;
	emissiveData.height = 1;
	emissiveData.width = 1;
	emissiveData.channels = 4;
	emissiveData.pixels = emissivePixel.data();
	createTexture(renderer,emissiveData, defaultEmissiveImage);

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

	DescriptorManager::createMaterialDescriptorPool(renderer.getLogicalDevice(),descriptorPool, scene.imageAssets.size());
	descriptorSets.reserve(scene.imageAssets.size());
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


void ResourceManager::createTexture(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, ImageResource& tex) const {
	createTextureImage(renderer,TEXTURE_PATH, tex.image);
	tex.imageView.createImageView(renderer.getLogicalDevice(), tex.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void ResourceManager::createTexture(const VulkanRenderer& renderer, const ImageAsset& data, ImageResource& tex) const {
	createTextureImage(renderer, data, tex.image);
	tex.imageView.createImageView(renderer.getLogicalDevice(), tex.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void ResourceManager::createTextureImage(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, VulkanImage& textureImage) const {
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
	createTextureImageHelper(renderer, pixels, texWidth, texHeight, textureImage);

	stbi_image_free(pixels);
}

void ResourceManager::createTextureImage(const VulkanRenderer& renderer, const ImageAsset& data, VulkanImage& textureImage)const {

	VkDeviceSize imageSize = data.width * data.height * 4;
	createTextureImageHelper(renderer, data.pixels, data.width, data.height, textureImage);


}


void ResourceManager::createTextureImageHelper(const VulkanRenderer& renderer, stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage) const {

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
	textureImageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;


	textureImage.create2DImage(renderer.getAllocator(), textureImageInfo);

	transitionImageLayout(renderer, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	copyBufferToImage(renderer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(renderer, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, renderer.getPhysicalDevice().findQueueFamilies(renderer.getSurface()).transferFamily.value()
		, renderer.getPhysicalDevice().findQueueFamilies(renderer.getSurface()).graphicsFamily.value());

	transitionImageLayout(renderer, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
}

void ResourceManager::createSampler(const VulkanRenderer& renderer, SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV) const {


	samplerResource.sampler.createTextureSampler(renderer.getPhysicalDevice(), renderer.getLogicalDevice(), magFilter, minFilter, mipMap, addressU, adressV);

}


void ResourceManager::transitionImageLayout(const VulkanRenderer& renderer, VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue)const {

	VulkanCommandBuffer commandBuffer;
	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
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
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

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