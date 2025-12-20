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


	void run() {

		mainLoop();

	}

	VulkanRenderer();
	~VulkanRenderer() = default;
	VulkanRenderer(const VulkanRenderer&) = default;


	void createMeshResources(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer);

	void createTexture(const std::string& TEXTURE_PATH, ImageResource& tex);

	void createTexture(const ImageArrayData& data, ImageResource& tex);

	void createSampler(SamplerResource& sampler, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV);

	const VulkanPhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
	VulkanLogicalDevice& getLogicalDevice() { return logicalDevice; }
	const VulkanSwapChain& getSwapChain() const { return swapChain; }
	const VulkanRenderPass& getRenderPass() const { return renderPass; }
	const VulkanFrameBuffers& getFrameBuffers() const { return frameBuffers; }
	const VulkanGraphicsPipeline& getGraphicsPipeline() const { return graphicsPipeline; }
	const VulkanCommandPool& getCommandPool() const { return commandPool; }
	const VulkanMemoryAllocator & getAllocator() const { return allocator; }

	void bufferStagedUpload(VulkanBuffer& dstBuffer,const void* bufferData, uint32_t size, uint32_t elementCount) const;
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

	ModelLoader											modelLoader;


	//Resources
	MeshBuffers											meshBuffer{};

	std::vector<VulkanBuffer>							uniformBuffers;

	std::vector<void*>									uniformBuffersMapped;

	ImageResource										textureResource;

	GltfLoader											sceneLoader;

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

	void createVertexBuffer(const std::vector<Vertex>& vertices, VulkanBuffer& buffer);

	void createIndexBuffer(const std::vector<uint32_t>& indices, VulkanBuffer& buffer);

	void createTextureImage(const std::string& TEXTURE_PATH, VulkanImage& textureImage);

	void createTextureImage(const ImageArrayData& data, VulkanImage& textureImage);

	void createTextureImageHelper(const stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage);

	void transitionImageLayout(VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue);

	void copyBufferToImage(VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height);

	void createDepthResources();

	void cleanDepthResources();

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const;

};

