#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>




#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


#include "../Includes/VulkanIncludes.h"
#include "../Platform/Window.h"
#include "../../Engine/Scene/Camera.h"

#include "DescriptorManager.h"
#include "CommandBufferRecorder.h"
#include "PipelineFactory.h"




static const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRenderer
{

public:


	void run(SceneFramesData& drawData, ResourceManager& resourceManager,Camera& camera) {

		
		glfwPollEvents();
		drawFrame(drawData,	resourceManager,camera);
		vkDeviceWaitIdle(logicalDevice.getDevice());
	}

	VulkanRenderer();
	~VulkanRenderer() = default;
	VulkanRenderer(const VulkanRenderer&) = default;



	bool running() { return !window.shouldClose(); }
	const VulkanPhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
	const VulkanLogicalDevice& getLogicalDevice() const { return logicalDevice; }
	VulkanLogicalDevice& getLogicalDevice() { return logicalDevice; }
	const VulkanSwapChain& getSwapChain() const { return swapChain; }
	const VulkanRenderPass& getRenderPass() const { return renderPass; }
	const VulkanFrameBuffers& getFrameBuffers() const { return frameBuffers; }
	const VulkanPipeline& getGraphicsPipeline() const { return graphicsPipeline; }
	const VulkanPipeline& getEnvironmentPipeline() const { return environmentPipeline; }
	const VulkanCommandPool& getCommandPool() const { return commandPool; }
	const VulkanCommandPool& getTransferCommandPool() const { return transferCommandPool; }
	const VulkanMemoryAllocator & getAllocator() const { return allocator; }
	const VulkanSurface& getSurface() const { return surface; }


	// cupe map creation for env and irradiance maps
	void cubePass(VulkanImageView& imgResource, CubeMapResource& cubeMap, VulkanSampler& sampler, MeshBuffers& cubeMesh, uint32_t texSize, bool irr) const;
	void prefilteredCubePass(VulkanImageView& imgResource, CubeMapResource& cubeMap, VulkanSampler& sampler, MeshBuffers& cubeMesh,const uint32_t texSize) const;

private:



	uint32_t currentFrame = 0;

	Window												window{};

	VulkanInstance										instance{};

	VulkanSurface										surface{};

	VulkanPhysicalDevice								physicalDevice{};

	VulkanLogicalDevice									logicalDevice{};

	VulkanMemoryAllocator 								allocator;

	VulkanSwapChain										swapChain{};

	VulkanRenderPass									renderPass{};

	std::vector<VulkanDescriptorSet>					descriptorSets;

	VulkanDescriptorPool								descriptorPool{};

	VulkanPipeline										graphicsPipeline{};

	VulkanPipeline										environmentPipeline{};

	VulkanFrameBuffers									frameBuffers{};

	VulkanCommandPool									commandPool{};

	VulkanCommandPool									transferCommandPool{};

	std::vector <VulkanCommandBuffer>					commandBuffers;

	VulkanSyncObjects									syncObjects{};



	//Resources


	std::vector<VulkanBuffer>							uniformBuffers;

	std::vector<void*>									uniformBuffersMapped;

	VulkanImage											depthImage;

	VulkanImageView										depthImageView;

	VulkanImage colorImage;

	VulkanImageView colorImageView;



	void initVulkan();

	//void mainLoop();

	void cleanSwapChain();

	void recreateSwapChain();

	void drawFrame(SceneFramesData& drawData, ResourceManager& resourceManager,Camera& camera);

	void processInput(float deltaTime);

	void createDepthResources(VkSampleCountFlagBits msaaSamples);

	void cleanDepthResources();

	void cleanColorResources();

	void update(uint32_t currentImage, int lightCount, Camera& camera);

	void updateUniformBuffer(uint32_t currentImage, float deltaTime, int lightCount, Camera& camera);

	void createUniformBuffers();

	void createColorResources(VkSampleCountFlagBits msaaSamples);

	uint32_t beginFrame();
	
	void envPass(uint32_t imageIndex, ResourceManager& resourceManager);

	void forwardPass(uint32_t imageIndex, SceneFramesData& drawData, ResourceManager& resourceManager);

	void presentFrame(uint32_t imageIndex);
};

