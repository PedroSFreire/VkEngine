
#include "..\headers\VulkanRenderer.h"
#include "..\headers\Scene.h"
#include "..\headers\ResourceManager.h"

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

	createUBODescriptorPool(descriptorPool, MAX_FRAMES_IN_FLIGHT);

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		createUBODescriptorLayout(descriptorSets[i], descriptorPool);
		descriptorSets[i].createDescriptor();
		updateUBODescriptor(descriptorSets[i],uniformBuffers[i]);
	}

	VulkanDescriptorSet matDescriptorSet,lightDescriptor;
	VulkanDescriptorPool matDescriptorPool, lightDescriptorPool;

	createMaterialDescriptorPool(matDescriptorPool, 1);
	createLightDescriptorPool(lightDescriptorPool, 1);

	createMaterialDescriptorLayout(matDescriptorSet, matDescriptorPool);
	createLightDescriptorLayout(lightDescriptor, lightDescriptorPool);

	std::array<VkDescriptorSetLayout, 3> descriptorLayouts{ descriptorSets[0].getDescriptorSetLayout(), matDescriptorSet.getDescriptorSetLayout(), lightDescriptor.getDescriptorSetLayout() };



	graphicsPipeline.createGraphicsPipeline(logicalDevice, swapChain, renderPass, descriptorLayouts.data());

	createDepthResources();

	frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass, depthImageView);
	
	syncObjects.createSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT, swapChain.getSwapChainImages().size());

}



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


void VulkanRenderer::drawFrame(Scene& scene, ResourceManager& resourceManager)
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


	vkResetCommandBuffer(commandBuffers[currentFrame].getCommandBuffer(), 0);
	update(currentFrame,scene.getScene());


	recordCommandBufferScene(commandBuffers[currentFrame],imageIndex, scene, descriptorSets[currentFrame].getDescriptorSet(),resourceManager);


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

void VulkanRenderer::processInput(float deltaTime) {
	// Implement camera movement and input processing here
	if (glfwGetKey(window.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window.getWindow(), true);
	double mouseX, mouseY;
	glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

	camera.updateCam(mouseX,mouseY);


	//WASD movement
	if (glfwGetKey(window.getWindow(), GLFW_KEY_W) == GLFW_PRESS)
		camera.goForward(2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
		camera.goForward(-2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_D) == GLFW_PRESS)
		camera.goRight(2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_A) == GLFW_PRESS)
		camera.goRight(-2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.goUp(1.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.goUp(-1.0f * deltaTime);


}

void VulkanRenderer::update(uint32_t currentImage, SceneData& sceneData) {
	static auto prevTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
	processInput(deltaTime);
	updateUniformBuffer(currentImage, deltaTime, sceneData);
	prevTime = currentTime;
}
void VulkanRenderer::updateUniformBuffer(uint32_t currentImage,float  deltaTime, SceneData& sceneData) {

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), /*time **/ glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.view = glm::lookAt(camera.getPos(), camera.getPos() + camera.getDirection(), camera.getUp());

	ubo.proj = glm::perspective(glm::radians(camera.getFov()), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, 0.01f, 2000.0f);

	ubo.proj[1][1] *= -1;

	ubo.lightCount = sceneData.lights.size();
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


void VulkanRenderer::createMaterialDescriptorLayout(VulkanDescriptorSet& set, VulkanDescriptorPool& pool) const {
	std::array<VkDescriptorSetLayoutBinding, 5> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].pImmutableSamplers = nullptr;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[3].pImmutableSamplers = nullptr;
	bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[4].pImmutableSamplers = nullptr;
	bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	set.createDescriptorLayout(logicalDevice, pool, bindings);
}
void VulkanRenderer::createUBODescriptorLayout(VulkanDescriptorSet& set, VulkanDescriptorPool& pool) const {


	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	set.createDescriptorLayout(logicalDevice, pool, bindings);
}
void VulkanRenderer::createLightDescriptorLayout( VulkanDescriptorSet& set, VulkanDescriptorPool& pool) const {
	
	std::array<VkDescriptorSetLayoutBinding, 1> bindings{};

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	set.createDescriptorLayout(logicalDevice, pool, bindings);
}

void VulkanRenderer::createMaterialDescriptorPool(VulkanDescriptorPool& pool, int size) const{
	DescriptorPoolCreateInfo info{};

	std::array<VkDescriptorPoolSize, 1> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size) * 5;

	info.poolSizes = poolSizes;
	info.maxSets = static_cast<uint32_t>(size);
	info.flags = 0;

	pool.createDescriptorPool(logicalDevice, info);
}
void VulkanRenderer::createUBODescriptorPool(VulkanDescriptorPool& pool, int size) const{

	DescriptorPoolCreateInfo info{};
	
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size);

	info.poolSizes = poolSizes;
	info.maxSets = static_cast<uint32_t>(size);
	info.flags = 0;

	pool.createDescriptorPool(logicalDevice, info);
}
void VulkanRenderer::createLightDescriptorPool(VulkanDescriptorPool& pool, size_t size) const{
	DescriptorPoolCreateInfo info{};

	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(size);

	info.poolSizes = poolSizes;
	info.maxSets = static_cast<uint32_t>(size);
	info.flags = 0;

	pool.createDescriptorPool(logicalDevice, info);
}

void VulkanRenderer::updateLightDescriptor(VulkanDescriptorSet& set,VulkanBuffer& lightBuffer, size_t numLights) const{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = lightBuffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;


	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = set.getDescriptorSet();
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	set.updateDescriptorSet(descriptorWrites);
}
void VulkanRenderer::updateMaterialDescriptor(VulkanDescriptorSet& set,const VkImageView* textureView, const VkSampler* textureSampler) const{
	
	VkDescriptorImageInfo colorInfo{};
	colorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorInfo.imageView = textureView[0];
	colorInfo.sampler = textureSampler[0];

	VkDescriptorImageInfo normalInfo{};
	normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalInfo.imageView = textureView[1];
	normalInfo.sampler = textureSampler[1];

	VkDescriptorImageInfo metalRoughInfo{};
	metalRoughInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	metalRoughInfo.imageView = textureView[2];
	metalRoughInfo.sampler = textureSampler[2];

	VkDescriptorImageInfo occlusionInfo{};
	occlusionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	occlusionInfo.imageView = textureView[3];
	occlusionInfo.sampler = textureSampler[3];

	VkDescriptorImageInfo emissiveInfo{};
	emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	emissiveInfo.imageView = textureView[4];
	emissiveInfo.sampler = textureSampler[4];



	std::array<VkWriteDescriptorSet, 5> descriptorWrites{};



	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = set.getDescriptorSet();
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &colorInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = set.getDescriptorSet();
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &normalInfo;


	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = set.getDescriptorSet();
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &metalRoughInfo;


	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].dstSet = set.getDescriptorSet();
	descriptorWrites[3].dstBinding = 3;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].pImageInfo = &occlusionInfo;


	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].dstSet = set.getDescriptorSet();
	descriptorWrites[4].dstBinding = 4;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].pImageInfo = &emissiveInfo;
	
	set.updateDescriptorSet(descriptorWrites);
}
void VulkanRenderer::updateUBODescriptor(VulkanDescriptorSet& set,VulkanBuffer& uniformBuffer) const {
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer.getBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);


	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = set.getDescriptorSet();
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;
	
	set.updateDescriptorSet(descriptorWrites);
}







//CommandBuffer this should be in its own thing
void VulkanRenderer::bindMesh(VulkanCommandBuffer&commandBuffer,const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer) {
	VkBuffer vertexBuffers[] = { vertBuffer.getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer.getCommandBuffer(), 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer.getCommandBuffer(), indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);


}

void VulkanRenderer::recordDrawCall(VulkanCommandBuffer& commandBuffer, const VulkanGraphicsPipeline& graphicsPipeline, const CPUDrawCallData data, uint32_t indexCount, ResourceManager& resourceManager, VkDescriptorSet currentUBO){
	std::array<VkDescriptorSet, 3> descriptor{ currentUBO,resourceManager.getDescriptor(data.materialDescriptorId).getDescriptorSet() ,resourceManager.getLightDescriptor().getDescriptorSet() };



	const pushConstants pushData{ data.transform ,data.mat->colorFactor,data.mat->metallicFactor,data.mat->roughnessFactor,data.mat->emissiveStrenght,0,data.mat->emissiveFactor,0 };

	vkCmdPushConstants(commandBuffer.getCommandBuffer(), graphicsPipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushData), &pushData);

	vkCmdBindDescriptorSets(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getPipelineLayout(), 0, 3, descriptor.data(), 0, nullptr);

	vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), indexCount, 1, 0, 0, 0);
}

void VulkanRenderer::recordCommandBufferScene(VulkanCommandBuffer& commandBuffer, const uint32_t imageIndex, Scene& scene, VkDescriptorSet currentUBO, ResourceManager& resourceManager){


	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;                   // Optional
	beginInfo.pInheritanceInfo = nullptr;  // Optional

	if (vkBeginCommandBuffer(commandBuffer.getCommandBuffer(), &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass.getRenderPass();
	renderPassInfo.framebuffer = frameBuffers.getFrameBufferAtIndex(imageIndex);
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();




	vkCmdBeginRenderPass(commandBuffer.getCommandBuffer(), &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline.getGraphicsPipeline());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChain.getSwapChainExtent().width);
	viewport.height = static_cast<float>(swapChain.getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer.getCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChain.getSwapChainExtent();
	vkCmdSetScissor(commandBuffer.getCommandBuffer(), 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getGraphicsPipeline());

	SceneFramesData& data = scene.recordScene();
	resourceManager.loadLights(*this, data.frameLightData);

	for (CPUDrawCallInstanceData& instance : data.drawInstances) {
		MeshBuffers& mesh = resourceManager.getMesh(instance.meshResourceId).meshBuffers;
		bindMesh(commandBuffer,mesh.vertexBuffer, mesh.indexBuffer);
		int indexCount = scene.getMeshAsset(instance.meshId).indices.size();
		for (CPUDrawCallData& drawCall : instance.cpuDrawCalls) {

			recordDrawCall(commandBuffer,graphicsPipeline, drawCall, indexCount, resourceManager,currentUBO);
		}
	}


	vkCmdEndRenderPass(commandBuffer.getCommandBuffer());

	if (vkEndCommandBuffer(commandBuffer.getCommandBuffer()) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanRenderer::recordCommandBufferCopyBuffer(VulkanCommandBuffer& commandBuffer, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize size){

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer.getCommandBuffer(), &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer.getCommandBuffer());

}


