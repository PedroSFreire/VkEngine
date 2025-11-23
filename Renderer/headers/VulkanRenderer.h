#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>




#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <fstream>



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
#include "VulkanMemoryAllocator.h"


#include <algorithm>
#include <cstdint>
#include <limits>
#include <vk_mem_alloc.h>

#include "Camera.h"
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

#include "ModelLoader.h"





static const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRenderer
{

public:


	void run() {
		initVulkan();
		mainLoop();

	}

	VulkanRenderer();

private:


	uint32_t currentFrame = 0;

	Camera												camera;

	Window												window{};

	VulkanInstance										instance{};

	VulkanSurface										surface{};

	VulkanPhysicalDevice								physicalDevice{};

	VulkanLogicalDevice									logicalDevice{};

	VulkanMemoryAllocator 								allocator;

	VulkanSwapChain										swapChain{};

	VulkanRenderPass									renderPass{};

	VulkanDescriptorSet									descriptorSet{};

	VulkanGraphicsPipeline								graphicsPipeline{};

	VulkanFrameBuffers									frameBuffers{};

	VulkanCommandPool									commandPool{};

	VulkanCommandPool									transferCommandPool{};

	std::vector <VulkanCommandBuffer>					commandBuffers;

	VulkanSyncObjects									syncObjects{};

	ModelLoader											modelLoader;

	//Resources

	VulkanBuffer										vertexBuffer{};

	VulkanBuffer										indexBuffer{};

	std::vector<VulkanBuffer>							uniformBuffers;

	std::vector<void*>									uniformBuffersMapped;

	VulkanImage											textureImage;

	VulkanImageView										textureImageView;

	VulkanImage											depthImage;

	VulkanImageView										depthImageView;

	VulkanSampler										textureSampler;



	const std::string MODEL_PATH = "models/viking_room.obj";
	const std::string TEXTURE_PATH = "textures/viking_room.png";




	void initVulkan();

	void mainLoop();

	void cleanSwapChain();

	void recreateSwapChain();

	void drawFrame();

	void processInput(float deltaTime);

	//resource creation and maintenance functions

	void createUniformBuffers();

	void update(uint32_t currentImage);

	void updateUniformBuffer(uint32_t currentImage, float deltaTime);

	void createVertexBuffer();

	void createIndexBuffer();

	void createTextureImage(const std::string& TEXTURE_PATH);

	void transitionImageLayout(VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue);

	void copyBufferToImage(VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height);

	void createDepthResources();

	void cleanDepthResources();

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size);

};

