#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <fstream>


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>


#include "Window.h"

#include "VulkanDebugHandler.h"
#include "VulkanInstance.h"

#include "VulkanSurface.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"


#include "VulkanSwapChain.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFrameBuffers.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSyncObjects.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};





class HelloTriangleApplication {
public:


	void run() {
		initVulkan();
		mainLoop();
		
	}

	HelloTriangleApplication() : commandBuffers(MAX_FRAMES_IN_FLIGHT) {
	}

private:


	static const int MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrame = 0;


	Window window{};
	VulkanInstance instance{};
	VulkanSurface surface{};
	VulkanPhysicalDevice physicalDevice{};
	VulkanLogicalDevice logicalDevice{};
	VulkanSwapChain swapChain{};
	VulkanRenderPass renderPass{};
	VulkanDescriptorSet descriptorSet{};
	VulkanGraphicsPipeline graphicsPipeline{};
	VulkanFrameBuffers frameBuffers{};
	VulkanCommandPool commandPool{};
	VulkanCommandPool transferCommandPool{};
	std::vector <VulkanCommandBuffer> commandBuffers;
	VulkanSyncObjects syncObjects{};
	VulkanBuffer vertexBuffer{};
	VulkanBuffer indexBuffer{};

	std::vector<VulkanBuffer> uniformBuffers;
	std::vector<void*> uniformBuffersMapped;


	VulkanImage textureImage;
	VulkanImageView textureImageView;

	VulkanImage depthImage;
	VulkanImageView depthImageView;
	
	VulkanSampler textureSampler;


	



	void initVulkan() {
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

		createTextureImage();

		textureImageView.createImageView(logicalDevice, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

		textureSampler.createTextureSampler(physicalDevice,logicalDevice);

		syncObjects.createSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT);

		createVertexBuffer();

		createIndexBuffer();

		createUniformBuffers();

		descriptorSet.createDescriptorPool(MAX_FRAMES_IN_FLIGHT);

		descriptorSet.createDescriptorSets(uniformBuffers,MAX_FRAMES_IN_FLIGHT,textureImageView, textureSampler);
	}



	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size) {
		VulkanCommandBuffer commandBuffer;
		commandBuffer.beginRecordindSingleTimeCommands(logicalDevice, transferCommandPool);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

		commandBuffer.endRecordingSingleTimeCommands(logicalDevice, transferCommandPool);

	}

	void createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		uint32_t vextexCount = static_cast<uint32_t>(vertices.size());

		VulkanBuffer stagingBuffer;
		stagingBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, vextexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


		void* data;

		vkMapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory(), 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory());

		vertexBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, vextexCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		copyBuffer(stagingBuffer.getBuffer(), vertexBuffer.getBuffer(), bufferSize);
	}

	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		uint32_t indexCount = static_cast<uint32_t>(indices.size());
		VulkanBuffer stagingBuffer;
		stagingBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		void* data;
		vkMapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory(), 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDevice.getDevice(), stagingBuffer.getBufferMemory());
		indexBuffer.createBuffer(physicalDevice, logicalDevice, bufferSize, indexCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		copyBuffer(stagingBuffer.getBuffer(), indexBuffer.getBuffer(), bufferSize);
	}



	void mainLoop() {
		while (!window.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(logicalDevice.getDevice());
	}


	void cleanSwapChain() {
		frameBuffers.clean();
		swapChain.clean();
	}

	void recreateSwapChain() {

		int width = 0, height = 0;
		glfwGetFramebufferSize(window.getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window.getWindow(), &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(logicalDevice.getDevice());

		cleanSwapChain();

		swapChain.createSwapChain(physicalDevice, logicalDevice, surface, window);
		swapChain.createImageViews();
		frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass,depthImageView);
	}



	void drawFrame()
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

		updateUniformBuffer(currentFrame);


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


		VkSemaphore signalSemaphores[] = { syncObjects.renderFinishedSemaphores[currentFrame] };
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


	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			uniformBuffers[i].createBuffer(physicalDevice, logicalDevice , bufferSize, 1 , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkMapMemory(logicalDevice.getDevice(), uniformBuffers[i].getBufferMemory() , 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}

	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(glm::radians(45.0f), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, 0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}


	void createTextureImage() {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
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


	
	void transitionImageLayout(VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue ) {

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

	void copyBufferToImage(VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height) {
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
	
	void createDepthResources() {
		VkFormat depthFormat = physicalDevice.findDepthFormat();

		depthImage.create2DImage(physicalDevice, logicalDevice, swapChain.getSwapChainExtent().width,
				swapChain.getSwapChainExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		
		depthImageView.createImageView(logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	}
	

};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}