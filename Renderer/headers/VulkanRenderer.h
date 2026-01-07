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


#include "Camera.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFrameBuffers.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSyncObjects.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorPool.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

#include "ModelLoader.h"
#include "GltfLoader.h"




static const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRenderer
{

public:


	void run(Scene& scene, ResourceManager& resourceManager) {

		
		glfwPollEvents();
		drawFrame(scene,	resourceManager);
		vkDeviceWaitIdle(logicalDevice.getDevice());
	}

	VulkanRenderer();
	~VulkanRenderer() = default;
	VulkanRenderer(const VulkanRenderer&) = default;


	void createMeshResources(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const;

	void createTexture(const std::string& TEXTURE_PATH, ImageResource& tex) const;

	void createTexture(const ImageAsset& data, ImageResource& tex) const;

	void createSampler(SamplerResource& sampler, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV) const;

	bool running() { return !window.shouldClose(); }
	const VulkanPhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
	VulkanLogicalDevice& getLogicalDevice() { return logicalDevice; }
	const VulkanLogicalDevice& getLogicalDevice() const { return logicalDevice; }
	const VulkanSwapChain& getSwapChain() const { return swapChain; }
	const VulkanRenderPass& getRenderPass() const { return renderPass; }
	const VulkanFrameBuffers& getFrameBuffers() const { return frameBuffers; }
	const VulkanGraphicsPipeline& getGraphicsPipeline() const { return graphicsPipeline; }
	const VulkanCommandPool& getCommandPool() const { return commandPool; }
	const VulkanMemoryAllocator & getAllocator() const { return allocator; }

	void bufferStagedUpload(VulkanBuffer& dstBuffer,const void* bufferData, uint32_t size, uint32_t elementCount) const;

	//The following functions are for descriptor set management and should be refactored into a DescriptorManager class later
	// *******************************************************************************************************************************************
	// Descriptor Pools
	void createMaterialDescriptorPool(VulkanDescriptorPool& pool, int size) const;
	void createUBODescriptorPool(VulkanDescriptorPool& pool, int size) const;
	void createLightDescriptorPool(VulkanDescriptorPool& pool, size_t size) const;


	// Descriptor Set Layouts
	void createMaterialDescriptorLayout(VulkanDescriptorSet& set, VulkanDescriptorPool& pool)const;
	void createUBODescriptorLayout(VulkanDescriptorSet& set, VulkanDescriptorPool& pool)const;
	void createLightDescriptorLayout(VulkanDescriptorSet& set, VulkanDescriptorPool& pool)const;

	// Descriptor Set Updates
	void updateLightDescriptor(VulkanDescriptorSet& set, VulkanBuffer& lightBuffer, size_t numLights)const;
	void updateMaterialDescriptor(VulkanDescriptorSet& set, const VkImageView* textureView, const VkSampler* textureSampler)const;
	void updateUBODescriptor(VulkanDescriptorSet& set, VulkanBuffer& uniformBuffer)const;
	// *******************************************************************************************************************************************
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

	std::vector<VulkanDescriptorSet>					descriptorSets;

	VulkanDescriptorPool								descriptorPool{};

	VulkanGraphicsPipeline								graphicsPipeline{};

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





	void initVulkan();

	//void mainLoop();

	void cleanSwapChain();

	void recreateSwapChain();

	void drawFrame(Scene& scene, ResourceManager& resourceManager);

	void processInput(float deltaTime);

	//resource creation and maintenance functions

	void createUniformBuffers();

	void update(uint32_t currentImage, SceneData& sceneData);

	void updateUniformBuffer(uint32_t currentImage, float deltaTime, SceneData& sceneData);

	void createVertexBuffer(const std::vector<Vertex>& vertices, VulkanBuffer& buffer) const;

	void createIndexBuffer(const std::vector<uint32_t>& indices, VulkanBuffer& buffer) const;

	void createTextureImage(const std::string& TEXTURE_PATH, VulkanImage& textureImage) const;

	void createTextureImage(const ImageAsset& data, VulkanImage& textureImage) const;

	void createTextureImageHelper(stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage) const;

	void transitionImageLayout(VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue) const;

	void copyBufferToImage(VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height) const;

	void createDepthResources();

	void cleanDepthResources();

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const;


	// commandBuffer recording and helpers**************************************************************
	void bindMesh(VulkanCommandBuffer& commandBuffer, const VulkanBuffer& vertBuffer, const VulkanBuffer& indexBuffer);

	void recordDrawCall(VulkanCommandBuffer& commandBuffer, const VulkanGraphicsPipeline& graphicsPipeline, const CPUDrawCallData data, uint32_t indexCount, ResourceManager& resourceManager,VkDescriptorSet currentUBO);

	void recordCommandBufferScene(VulkanCommandBuffer& commandBuffer, const uint32_t imageIndex, Scene& scene, VkDescriptorSet descriptorSet, ResourceManager& resourceManager);

	void recordCommandBufferCopyBuffer(VulkanCommandBuffer& commandBuffer, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize size);

	

	//**************************************************************************************************

};

