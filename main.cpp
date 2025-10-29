#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <fstream>


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
#include "VulkanCommandBuffer.h"
#include "VulkanSyncObjects.h"



class HelloTriangleApplication {
public:


	void run() {
		initVulkan();
		mainLoop();
	}

private:


	const int MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrame = 0;


	Window window{};
	VulkanInstance instance{};
	VulkanSurface surface{};
	VulkanPhysicalDevice physicalDevice{};
	VulkanLogicalDevice logicalDevice{};
	VulkanSwapChain swapChain{};
	VulkanRenderPass renderPass{};
	VulkanGraphicsPipeline graphicsPipeline{};
	VulkanFrameBuffers frameBuffers{};
	VulkanCommandBuffer commandBuffers{};
	VulkanSyncObjects syncObjects{};







	void initVulkan() {
		//instance.vulkanInstanceCreator();

		surface.createSurface(instance, window);

		physicalDevice.pickPhysicalDevice(instance, surface);

		logicalDevice.createLogicalDevice(instance, physicalDevice, surface);

		swapChain.createSwapChain(physicalDevice, logicalDevice, surface, window);

		swapChain.createImageViews();

		renderPass.createRenderPass(swapChain, logicalDevice);

		graphicsPipeline.createGraphicsPipeline(logicalDevice, swapChain, renderPass);

		frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass);

		commandBuffers.createCommandPool(physicalDevice, logicalDevice, surface);

		commandBuffers.createCommandBuffer(logicalDevice,MAX_FRAMES_IN_FLIGHT);

		syncObjects.createSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT);
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
		frameBuffers.createFramebuffers(logicalDevice, swapChain, renderPass);
	}



	void drawFrame()
	{
		uint32_t imageIndex;
		vkWaitForFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		
		VkResult result = vkAcquireNextImageKHR(logicalDevice.getDevice(), swapChain.getSwapChain(), UINT64_MAX, syncObjects.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS ) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(logicalDevice.getDevice(), 1, &syncObjects.inFlightFences[currentFrame]);

		vkResetCommandBuffer(commandBuffers.getCommandBuffer(currentFrame), 0);
		commandBuffers.recordCommandBuffer(imageIndex,  currentFrame, logicalDevice,  swapChain,  graphicsPipeline,  renderPass, frameBuffers);


		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { syncObjects.imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers.getCommandBuffer(currentFrame);
		

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