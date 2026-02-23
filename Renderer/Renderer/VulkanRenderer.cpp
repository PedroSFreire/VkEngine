#include "VulkanRenderer.h"


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

	renderPass.createForwardRenderPass(physicalDevice, logicalDevice, swapChain);

	DescriptorManager::createUBODescriptorPool(logicalDevice, descriptorPool, MAX_FRAMES_IN_FLIGHT);

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		DescriptorManager::createUBODescriptorLayout(logicalDevice,descriptorSets[i], descriptorPool);
		descriptorSets[i].createDescriptor();
		DescriptorManager::updateUBODescriptor(descriptorSets[i],uniformBuffers[i]);
	}

	VulkanDescriptorSet matDescriptorSet,lightDescriptor,cubeDescriptor;
	VulkanDescriptorPool matDescriptorPool, lightDescriptorPool, cubeDescriptorPool;

	DescriptorManager::createMaterialDescriptorPool(logicalDevice, matDescriptorPool, 1);
	DescriptorManager::createLightDescriptorPool(logicalDevice, lightDescriptorPool, 1);
	DescriptorManager::createCubeDescriptorPool(logicalDevice, cubeDescriptorPool, 1);

	DescriptorManager::createMaterialDescriptorLayout(logicalDevice, matDescriptorSet, matDescriptorPool);
	DescriptorManager::createLightDescriptorLayout(logicalDevice, lightDescriptor, lightDescriptorPool);
	DescriptorManager::createCubeDescriptorLayout(logicalDevice, cubeDescriptor, cubeDescriptorPool);

	std::array<VkDescriptorSetLayout, 4> descriptorLayouts{ descriptorSets[0].getDescriptorSetLayout(), matDescriptorSet.getDescriptorSetLayout(), lightDescriptor.getDescriptorSetLayout(), cubeDescriptor.getDescriptorSetLayout()};



	PipelineFactory::createGraphicsPipeline(graphicsPipeline ,logicalDevice, swapChain, renderPass, descriptorLayouts.data(), physicalDevice.getMsaaSamples());

	std::array<VkDescriptorSetLayout, 2> envDescriptorLayouts{ descriptorSets[0].getDescriptorSetLayout(), cubeDescriptor.getDescriptorSetLayout() };

	PipelineFactory::createEnvPipeline(environmentPipeline, logicalDevice, swapChain, renderPass, envDescriptorLayouts.data(), physicalDevice.getMsaaSamples());
	

	createColorResources(physicalDevice.getMsaaSamples());

	createDepthResources(physicalDevice.getMsaaSamples());

	frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass, depthImageView,colorImageView);
	
	syncObjects.createSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT, swapChain.getSwapChainImages().size());

}


void VulkanRenderer::createColorResources(VkSampleCountFlagBits msaaSamples) {
	VkFormat colorFormat = swapChain.getSwapChainImageFormat();

	VulkanImageCreateInfo colorImageInfo{};
	colorImageInfo.width = swapChain.getSwapChainExtent().width;
	colorImageInfo.height = swapChain.getSwapChainExtent().height;
	colorImageInfo.numSamples = msaaSamples;
	colorImageInfo.format = colorFormat;
	colorImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	colorImageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	colorImageInfo.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
	
	colorImage.create2DImage(allocator, colorImageInfo);
	colorImageView.createImageView(logicalDevice, colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}


void VulkanRenderer::createDepthResources(VkSampleCountFlagBits msaaSamples) {
	VkFormat depthFormat = physicalDevice.findDepthFormat();
	VulkanImageCreateInfo depthImageInfo{};
	depthImageInfo.width = swapChain.getSwapChainExtent().width;
	depthImageInfo.height = swapChain.getSwapChainExtent().height;	
	depthImageInfo.numSamples = msaaSamples;
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

void VulkanRenderer::cleanColorResources() {
	colorImageView.clean();
	colorImage.clean();
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
	cleanColorResources();

	swapChain.createSwapChain(physicalDevice, logicalDevice, surface, window);
	createColorResources(physicalDevice.getMsaaSamples());
	createDepthResources(physicalDevice.getMsaaSamples());
	swapChain.createImageViews();
	frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass, depthImageView,colorImageView);
}


uint32_t VulkanRenderer::beginFrame() {
	uint32_t imageIndex;
	vkWaitForFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);


	VkResult result = vkAcquireNextImageKHR(logicalDevice.getDevice(), swapChain.getSwapChain(), UINT64_MAX, syncObjects.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();

	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[currentFrame]);

	vkResetCommandBuffer(commandBuffers[currentFrame].getCommandBuffer(), 0);

	return imageIndex;
}




void VulkanRenderer::forwardPass(uint32_t imageIndex, SceneFramesData& drawData,ResourceManager& resourceManager) {
	CommandBufferRecorder::recordCommandBufferForwardPass(*this, commandBuffers[currentFrame], imageIndex, drawData, descriptorSets[currentFrame].getDescriptorSet(), resourceManager);


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
}



void VulkanRenderer::presentFrame(uint32_t imageIndex) {
	VkSemaphore signalSemaphores[] = { syncObjects.renderFinishedSemaphores[imageIndex] };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain.getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(logicalDevice.getPresentQueue(), &presentInfo);


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.getFramebufferResized()) {
		window.setFramebufferResized(false);
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}


}



void VulkanRenderer::drawFrame(SceneFramesData& drawData, ResourceManager& resourceManager,Camera& camera)
{
	//acquire image from swap chain and wait and reset sync objects and frame resources
	uint32_t imageIndex = beginFrame();

	update(currentFrame, drawData.frameLightData.size(), camera);



	forwardPass(imageIndex, drawData, resourceManager);

	//present image to swap chain	
	presentFrame(imageIndex);

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


	ubo.view = glm::lookAt(camera.getPos(), camera.getPos() + camera.getDirection(), camera.getUp());

	ubo.proj = glm::perspective(glm::radians(camera.getFov()), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, 0.01f, 2000.0f);

	ubo.proj[1][1] *= -1;

	ubo.cameraPos = camera.getPos();

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





void VulkanRenderer::cubePass(VulkanImageView& imgResource, CubeMapResource& cubeMap, VulkanSampler& sampler, MeshBuffers& cubeMesh, uint32_t texSize, bool irr) const {


	//view for each face of the cubemap
	const std::array<glm::mat4, 6> cubemapLookAt = {
	glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};


	//create render pass 
	VulkanRenderPass cubeRenderPass;
	cubeRenderPass.createCubeRenderPass(physicalDevice, logicalDevice);



	//create framebuffers 
	VulkanFrameBuffers frameBuffers;
	frameBuffers.createCubeFramebuffers(logicalDevice, cubeRenderPass, cubeMap.imageViews, texSize);

	//thsi should be removed can just use texDescriptorSet
	//create descriptor set 
	VulkanDescriptorPool descriptorPool;
	VulkanDescriptorSet descriptorSet;

	DescriptorManager::createCubeDescriptorPool(logicalDevice, descriptorPool, 1);
	DescriptorManager::createCubeDescriptorLayout(logicalDevice, descriptorSet, descriptorPool);
	descriptorSet.createDescriptor();
	DescriptorManager::updateCubeDescriptor(descriptorSet, imgResource.getImageView(), sampler.getSampler());

	//create pipeline 
	VulkanPipeline pipeline;

	PipelineFactory::createCubePipeline(logicalDevice, pipeline, cubeRenderPass, descriptorSet.getDescriptorSetLayout(), texSize, irr);


	//create descriptor
	VulkanDescriptorPool texDescriptorPool;
	VulkanDescriptorSet texDescriptorSet;

	DescriptorManager::createCubeDescriptorPool(logicalDevice, texDescriptorPool, 1);
	DescriptorManager::createCubeDescriptorLayout(logicalDevice, texDescriptorSet, texDescriptorPool);
	texDescriptorSet.createDescriptor();

	DescriptorManager::updateCubeDescriptor(texDescriptorSet, imgResource.getImageView(), sampler.getSampler());

	//record command buffer
	VulkanCommandBuffer commandBuffer;
	VulkanCommandPool commandPool;
	commandPool.createGraphicsCommandPool(physicalDevice, logicalDevice, surface);
	commandBuffer.createCommandBuffer(logicalDevice, commandPool);

	CommandBufferRecorder::recordCommandBufferCubePass(*this, cubeRenderPass, frameBuffers, commandBuffer, pipeline, texSize, texDescriptorSet, cubeMesh);

	//submit command Buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(commandBuffer.getCommandBuffer());

	vkResetFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[0]);

	if (vkQueueSubmit(logicalDevice.getGraphicsQueue(), 1, &submitInfo, syncObjects.inFlightFences[0]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	//wait for completion
	vkWaitForFences(logicalDevice.getDevice(), 1, &(syncObjects.inFlightFences[0]), VK_TRUE, UINT64_MAX);


}




void VulkanRenderer::prefilteredCubePass(VulkanImageView& imgResource, CubeMapResource& cubeMap, VulkanSampler& sampler, MeshBuffers& cubeMesh,const uint32_t texSize) const {


	//view for each face of the cubemap
	const std::array<glm::mat4, 6> cubemapLookAt = {
	glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	const int levelCount = log2(texSize) + 1;
	// create image view for framebuffer becaus eneed one for each mip level and layer of the cubemap
	std::vector<CubeMapResource> framebufferImageViews;
	for (uint32_t i = 0; i < levelCount; i++) {
		CubeMapResource cubeResource;
		for (uint32_t j = 0; j < 6; j++) {
			VulkanImageView imageView;
			imageView.createImageView(logicalDevice, cubeMap.image, j, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT,i);
			cubeResource.imageViews[j] = std::move(imageView);
		}
		framebufferImageViews.push_back(std::move(cubeResource));
	}


	//create render pass 
	VulkanRenderPass cubeRenderPass;
	cubeRenderPass.createCubeRenderPass(physicalDevice, logicalDevice);



	//create framebuffersf
	std::vector<VulkanFrameBuffers> frameBuffers;
	for (uint32_t i = 0; i < levelCount; i++) {
		VulkanFrameBuffers frameBuffer;
		frameBuffer.createCubeFramebuffers(logicalDevice, cubeRenderPass, framebufferImageViews[i].imageViews, texSize >> i);
		frameBuffers.push_back(std::move(frameBuffer));
	}




	//create descriptor
	VulkanDescriptorPool texDescriptorPool;
	VulkanDescriptorSet texDescriptorSet;

	DescriptorManager::createCubeDescriptorPool(logicalDevice, texDescriptorPool, 1);
	DescriptorManager::createCubeDescriptorLayout(logicalDevice, texDescriptorSet, texDescriptorPool);
	texDescriptorSet.createDescriptor();

	DescriptorManager::updateCubeDescriptor(texDescriptorSet, imgResource.getImageView(), sampler.getSampler());


	//create pipeline 
	VulkanPipeline pipeline;

	PipelineFactory::createPreFilteredPipeline(logicalDevice, pipeline, cubeRenderPass, texDescriptorSet.getDescriptorSetLayout(), texSize);


	//record command buffer
	std::vector<VulkanCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> vkCommandBuffers;
	commandBuffers.resize(levelCount);
	VulkanCommandPool commandPool;
	commandPool.createGraphicsCommandPool(physicalDevice, logicalDevice, surface);
	for (int i = 0; i < levelCount; i++) {
		commandBuffers[i].createCommandBuffer(logicalDevice, commandPool);
		float roughness = (float)i / (float)(levelCount - 1);
		CommandBufferRecorder::recordCommandBufferPreFilteredCubePass(*this, cubeRenderPass, frameBuffers[i], commandBuffers[i], pipeline, texSize >>i, texDescriptorSet, cubeMesh, roughness);
		vkCommandBuffers.push_back(commandBuffers[i].getCommandBuffer());
	}


	//submit command Buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = levelCount;
	submitInfo.pCommandBuffers = vkCommandBuffers.data();

	vkResetFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[0]);

	if (vkQueueSubmit(logicalDevice.getGraphicsQueue(), 1, &submitInfo, syncObjects.inFlightFences[0]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	//wait for completion
	vkWaitForFences(logicalDevice.getDevice(), 1, &(syncObjects.inFlightFences[0]), VK_TRUE, UINT64_MAX);


}