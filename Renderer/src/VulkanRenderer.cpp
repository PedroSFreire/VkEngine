#include "..\headers\VulkanRenderer.h"

#define GLM_FORCE_RADIANS
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define STB_IMAGE_IMPLEMENTATION


#include <stb_image.h>
#include <GLFW/glfw3native.h>
#include "tiny_obj_loader.h"

VulkanRenderer::VulkanRenderer() : commandBuffers(MAX_FRAMES_IN_FLIGHT) {
}



void VulkanRenderer::initVulkan() {
	//instance.vulkanInstanceCreator();

	surface.createSurface(instance, window);

	physicalDevice.pickPhysicalDevice(instance, surface);

	logicalDevice.createLogicalDevice(instance, physicalDevice, surface);

	swapChain.createSwapChain(physicalDevice, logicalDevice, surface, window);

	swapChain.createImageViews();

	renderPass.createRenderPass(physicalDevice, swapChain, logicalDevice);

	descriptorSet.createDescriptorSetLayout(logicalDevice);

	graphicsPipeline.createGraphicsPipeline(logicalDevice, swapChain, renderPass, descriptorSet.getDescriptorSetLayout());

	createDepthResources();

	frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass, depthImageView);

	commandPool.createGraphicsCommandPool(physicalDevice, logicalDevice, surface);

	transferCommandPool.createTransferCommandPool(physicalDevice, logicalDevice, surface);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		commandBuffers[i].createCommandBuffer(logicalDevice, commandPool);
	}

	modelLoader.loadModel(MODEL_PATH);

	createTextureImage(TEXTURE_PATH);

	textureImageView.createImageView(logicalDevice, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	textureSampler.createTextureSampler(physicalDevice, logicalDevice);
	
	syncObjects.createSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT, swapChain.getSwapChainImages().size());

	createVertexBuffer();

	createIndexBuffer();

	createUniformBuffers();

	descriptorSet.createDescriptorPool(MAX_FRAMES_IN_FLIGHT);

	descriptorSet.createDescriptorSets(uniformBuffers, MAX_FRAMES_IN_FLIGHT, textureImageView, textureSampler);
}



void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) {
	VulkanCommandBuffer commandBuffer;
	commandBuffer.beginRecordindSingleTimeCommands(logicalDevice, transferCommandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

	commandBuffer.endRecordingSingleTimeCommands(logicalDevice, transferCommandPool);

}

void VulkanRenderer::createVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(modelLoader.getVertices()[0]) * modelLoader.getVertices().size();
	uint32_t vextexCount = static_cast<uint32_t>(modelLoader.getVertices().size());

	VulkanBuffer stagingBuffer;
	stagingBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, vextexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


	void* data;

	vkMapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory(), 0, bufferSize, 0, &data);
	memcpy(data, modelLoader.getVertices().data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory());

	vertexBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, vextexCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(stagingBuffer.getBuffer(), vertexBuffer.getBuffer(), bufferSize);
}

void VulkanRenderer::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(modelLoader.getIndices()[0]) * modelLoader.getIndices().size();
	uint32_t indexCount = static_cast<uint32_t>(modelLoader.getIndices().size());
	VulkanBuffer stagingBuffer;
	stagingBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	vkMapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory(), 0, bufferSize, 0, &data);
	memcpy(data, modelLoader.getIndices().data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory());
	indexBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, indexCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	copyBuffer(stagingBuffer.getBuffer(), indexBuffer.getBuffer(), bufferSize);
}



void VulkanRenderer::mainLoop() {
	while (!window.shouldClose()) {
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(logicalDevice.getDevice());
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



void VulkanRenderer::drawFrame()
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
	commandBuffers[currentFrame].recordCommandBuffer(imageIndex, logicalDevice, swapChain, graphicsPipeline, renderPass, frameBuffers, vertexBuffer, indexBuffer, descriptorSet.getDescriptorSets(currentFrame));

	update(currentFrame);


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


void VulkanRenderer::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		uniformBuffers[i].createBuffer(physicalDevice, logicalDevice, bufferSize, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkMapMemory(logicalDevice.getDevice(), uniformBuffers[i].getBufferMemory(), 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
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

void VulkanRenderer::update(uint32_t currentImage) {
	static auto prevTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
	processInput(deltaTime);
	updateUniformBuffer(currentImage, deltaTime);
	prevTime = currentTime;
}
void VulkanRenderer::updateUniformBuffer(uint32_t currentImage,float  deltaTime) {

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), /*time **/ glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.view = glm::lookAt(camera.getPos(), camera.getPos() + camera.getDirection(), camera.getUp());

	ubo.proj = glm::perspective(glm::radians(45.0f), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, 0.1f, 10.0f);

	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}


void VulkanRenderer::createTextureImage(const std::string& TEXTURE_PATH) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VulkanBuffer stagingBuffer;
	stagingBuffer.createBuffer(physicalDevice, logicalDevice, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory(), 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory());

	stbi_image_free(pixels);

	textureImage.create2DImage(physicalDevice, logicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);

	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, physicalDevice.findQueueFamilies(surface).transferFamily.value()
		, physicalDevice.findQueueFamilies(surface).graphicsFamily.value());

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
}



void VulkanRenderer::transitionImageLayout(VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue) {

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

	//barrier.srcQueueFamilyIndex = physicalDevice.findQueueFamilies(surface).transferFamily.value();
	//barrier.dstQueueFamilyIndex = physicalDevice.findQueueFamilies(surface).graphicsFamily.value();

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

void VulkanRenderer::copyBufferToImage(VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height) {
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

	depthImage.create2DImage(physicalDevice, logicalDevice, swapChain.getSwapChainExtent().width,
		swapChain.getSwapChainExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	depthImageView.createImageView(logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void VulkanRenderer::cleanDepthResources() {
	depthImageView.clean();
	depthImage.clean();
}
