
#include "..\headers\VulkanRenderer.h"
#include "..\headers\Scene.h"
#include "..\headers\ResourceManager.h"
#include "..\headers\PipelineFactory.h"

#define GLM_FORCE_RADIANS
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define STB_IMAGE_IMPLEMENTATION

#include <omp.h>
#include <stb_image.h>
#include <GLFW/glfw3native.h>
#include "tiny_obj_loader.h"




#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

VulkanRenderer::VulkanRenderer() : commandBuffers(MAX_FRAMES_IN_FLIGHT) {
	initVulkan();
}



void VulkanRenderer::initVulkan() {

	surface.createSurface(instance, window);

	physicalDevice.pickPhysicalDevice(instance, surface);

	logicalDevice.createLogicalDevice(instance, physicalDevice, surface);

	allocator.createVmaAllocator(instance, physicalDevice, logicalDevice);

	swapChain.createSwapChain(physicalDevice, logicalDevice, surface, window);

	swapChain.createImageViews();

	createUniformBuffers();

	commandPool.createGraphicsCommandPool(physicalDevice, logicalDevice, surface);

	transferCommandPool.createTransferCommandPool(physicalDevice, logicalDevice, surface);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		commandBuffers[i].createCommandBuffer(logicalDevice, commandPool);
	}

	renderPass.createRenderPass(physicalDevice, swapChain, logicalDevice);

	DescriptorManager::createUBODescriptorPool(logicalDevice, descriptorPool, MAX_FRAMES_IN_FLIGHT);

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		DescriptorManager::createUBODescriptorLayout(logicalDevice,descriptorSets[i], descriptorPool);
		descriptorSets[i].createDescriptor();
		DescriptorManager::updateUBODescriptor(descriptorSets[i],uniformBuffers[i]);
	}

	VulkanDescriptorSet matDescriptorSet,lightDescriptor;
	VulkanDescriptorPool matDescriptorPool, lightDescriptorPool;

	DescriptorManager::createMaterialDescriptorPool(logicalDevice, matDescriptorPool, 1);
	DescriptorManager::createLightDescriptorPool(logicalDevice, lightDescriptorPool, 1);

	DescriptorManager::createMaterialDescriptorLayout(logicalDevice, matDescriptorSet, matDescriptorPool);
	DescriptorManager::createLightDescriptorLayout(logicalDevice, lightDescriptor, lightDescriptorPool);

	std::array<VkDescriptorSetLayout, 3> descriptorLayouts{ descriptorSets[0].getDescriptorSetLayout(), matDescriptorSet.getDescriptorSetLayout(), lightDescriptor.getDescriptorSetLayout() };



	PipelineFactory::createGraphicsPipeline(graphicsPipeline ,logicalDevice, swapChain, renderPass, descriptorLayouts.data());

	createDepthResources();

	frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass, depthImageView);
	
	syncObjects.createSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT, swapChain.getSwapChainImages().size());

}



void VulkanRenderer::createDepthResources() {
	VkFormat depthFormat = physicalDevice.findDepthFormat();
	VulkanImageCreateInfo depthImageInfo{};
	depthImageInfo.width = swapChain.getSwapChainExtent().width;
	depthImageInfo.height = swapChain.getSwapChainExtent().height;	
	depthImageInfo.format = depthFormat;	
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	depthImage.create2DImage(allocator,depthImageInfo);

	depthImageView.createImageView(logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void VulkanRenderer::cleanDepthResources() {
	depthImageView.clean();
	depthImage.clean();
}



void VulkanRenderer::cleanSwapChain() {
	frameBuffers.clean();
	swapChain.clean();
}

void VulkanRenderer::recreateSwapChain() {

	int width = 0, height = 0;
	glfwGetFramebufferSize(window.getWindow(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window.getWindow(), &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(logicalDevice.getDevice());

	cleanSwapChain();
	cleanDepthResources();


	swapChain.createSwapChain(physicalDevice, logicalDevice, surface, window);
	createDepthResources();
	swapChain.createImageViews();
	frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass, depthImageView);
}


void VulkanRenderer::drawFrame(SceneFramesData& drawData, ResourceManager& resourceManager,Camera& camera)
{
	uint32_t imageIndex;
	vkWaitForFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);


	VkResult result = vkAcquireNextImageKHR(logicalDevice.getDevice(), swapChain.getSwapChain(), UINT64_MAX, syncObjects.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[currentFrame]);

	resourceManager.getLightDescriptor();
	vkResetCommandBuffer(commandBuffers[currentFrame].getCommandBuffer(), 0);
	update(currentFrame, drawData.frameLightData.size(), camera);

	CommandBufferRecorder::recordCommandBufferBase(*this,commandBuffers[currentFrame],imageIndex, drawData, descriptorSets[currentFrame].getDescriptorSet(),resourceManager);


	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { syncObjects.imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame].getCommandBuffer();


	VkSemaphore signalSemaphores[] = { syncObjects.renderFinishedSemaphores[imageIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(logicalDevice.getGraphicsQueue(), 1, &submitInfo, syncObjects.inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain.getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(logicalDevice.getPresentQueue(), &presentInfo);


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.getFramebufferResized()) {
		window.setFramebufferResized(false);
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}



void VulkanRenderer::update(uint32_t currentImage, int lightCount, Camera& camera) {
	static auto prevTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
	camera.processInput(deltaTime, window);
	updateUniformBuffer(currentImage, deltaTime, lightCount, camera);
	prevTime = currentTime;
}
void VulkanRenderer::updateUniformBuffer(uint32_t currentImage,float  deltaTime, int lightCount, Camera& camera) {

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), /*time **/ glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.view = glm::lookAt(camera.getPos(), camera.getPos() + camera.getDirection(), camera.getUp());

	ubo.proj = glm::perspective(glm::radians(camera.getFov()), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, 0.01f, 2000.0f);

	ubo.proj[1][1] *= -1;

	ubo.lightCount = lightCount;
	memcpy(uniformBuffers[currentImage].getAllocationInfo().pMappedData, &ubo, sizeof(ubo));
}

void VulkanRenderer::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VulkanBufferCreateInfo bufferInfo{};
		bufferInfo.size = bufferSize;
		bufferInfo.elementCount = 1;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.vmaUsage = VMA_MEMORY_USAGE_AUTO;
		bufferInfo.vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT| VMA_ALLOCATION_CREATE_MAPPED_BIT;

		uniformBuffers[i].createBuffer(allocator, bufferInfo);
	}
}


/*

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const {
	VulkanCommandBuffer commandBuffer;
	commandBuffer.beginRecordindSingleTimeCommands(logicalDevice, transferCommandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

	commandBuffer.endRecordingSingleTimeCommands(logicalDevice, transferCommandPool);

}

void VulkanRenderer::createMeshResources(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const {

	createVertexBuffer(vertices,meshBuffer.vertexBuffer);
	createIndexBuffer(indices, meshBuffer.indexBuffer);

}

void VulkanRenderer::createVertexBuffer(const std::vector<Vertex>& vertices, VulkanBuffer& vertexBuffer) const {

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	uint32_t vextexCount = static_cast<uint32_t>(vertices.size());

	//create buffer
	VulkanBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.elementCount = vextexCount;
	vertexBufferInfo.size = bufferSize;
	vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	vertexBuffer.createBuffer(allocator,vertexBufferInfo);

	//upload data
	bufferStagedUpload(vertexBuffer, vertices.data(), bufferSize, vextexCount);

}

void VulkanRenderer::createIndexBuffer(const std::vector<uint32_t>& indices, VulkanBuffer& indexBuffer) const {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	uint32_t indexCount = static_cast<uint32_t>(indices.size());
	VulkanBuffer stagingBuffer;

	//create buffer
	VulkanBufferCreateInfo indexBufferInfo{};
	indexBufferInfo.elementCount = indexCount;
	indexBufferInfo.size = bufferSize;
	indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	indexBuffer.createBuffer(allocator, indexBufferInfo);

	//upload data
	bufferStagedUpload(indexBuffer, indices.data(), bufferSize, indexCount);
}

void VulkanRenderer::bufferStagedUpload(VulkanBuffer& dstBuffer,const void* bufferData, uint32_t size,uint32_t elementCount) const {

	VkDeviceSize bufferSize = size;


	VulkanBuffer stagingBuffer;

	VulkanBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.elementCount = elementCount;
	stagingBufferInfo.size = bufferSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.vmaUsage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferInfo.vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	stagingBuffer.createBuffer(allocator, stagingBufferInfo);


	void* data;
	vmaMapMemory(allocator.getAllocator(), stagingBuffer.getAllocation(), &data);
	memcpy(data, bufferData, (size_t)bufferSize);
	vmaUnmapMemory(allocator.getAllocator(), stagingBuffer.getAllocation());


	copyBuffer(stagingBuffer.getBuffer(), dstBuffer.getBuffer(), bufferSize);
}


void VulkanRenderer::createTexture(const std::string& TEXTURE_PATH, ImageResource& tex) const {
	createTextureImage(TEXTURE_PATH, tex.image);
	tex.imageView.createImageView(logicalDevice, tex.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanRenderer::createTexture(const ImageAsset& data, ImageResource& tex) const {
	createTextureImage(data, tex.image);
	tex.imageView.createImageView(logicalDevice, tex.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanRenderer::createTextureImage(const std::string& TEXTURE_PATH,VulkanImage& textureImage) const {
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels =stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
	createTextureImageHelper(pixels, texWidth, texHeight, textureImage);

	stbi_image_free(pixels);
}

void VulkanRenderer::createTextureImage(const ImageAsset& data, VulkanImage& textureImage)const {
	
	VkDeviceSize imageSize = data.width * data.height * 4;
	createTextureImageHelper(data.pixels, data.width, data.height, textureImage);

	
}


void VulkanRenderer::createTextureImageHelper( stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage) const {

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VulkanBuffer stagingBuffer;

	VulkanBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.elementCount = 1;
	stagingBufferInfo.size = imageSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.vmaUsage = VMA_MEMORY_USAGE_AUTO;
	stagingBufferInfo.vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;


	stagingBuffer.createBuffer(allocator, stagingBufferInfo);

	void* data;

	vmaMapMemory(allocator.getAllocator(), stagingBuffer.getAllocation(), &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vmaUnmapMemory(allocator.getAllocator(), stagingBuffer.getAllocation());

	VulkanImageCreateInfo textureImageInfo{};
	textureImageInfo.width = static_cast<uint32_t>(texWidth);
	textureImageInfo.height = static_cast<uint32_t>(texHeight);
	textureImageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;


	textureImage.create2DImage(allocator, textureImageInfo);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, physicalDevice.findQueueFamilies(surface).transferFamily.value()
		, physicalDevice.findQueueFamilies(surface).graphicsFamily.value());

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
}

void VulkanRenderer::createSampler(SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV) const{


	samplerResource.sampler.createTextureSampler(physicalDevice, logicalDevice, magFilter, minFilter, mipMap, addressU, adressV);

}


void VulkanRenderer::transitionImageLayout(VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue)const {

	VulkanCommandBuffer commandBuffer;
	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		commandBuffer.beginRecordindSingleTimeCommands(logicalDevice, commandPool);
	}
	else {
		commandBuffer.beginRecordindSingleTimeCommands(logicalDevice, transferCommandPool);
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

		commandBuffer.endRecordingSingleTimeCommands(logicalDevice, transferCommandPool);
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

		commandBuffer.endRecordingSingleTimeCommands(logicalDevice, transferCommandPool);
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
		commandBuffer.endRecordingSingleTimeCommands(logicalDevice, commandPool);
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}



}

void VulkanRenderer::copyBufferToImage(VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height)const {
	VulkanCommandBuffer commandBuffer;
	commandBuffer.beginRecordindSingleTimeCommands(logicalDevice, transferCommandPool);

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

	commandBuffer.endRecordingSingleTimeCommands(logicalDevice, transferCommandPool);
}


*/