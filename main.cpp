#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "VulkanDebugHandler.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "Window.h"
#include "VulkanSurface.h"
#include "VulkanLogicalDevice.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <cstdint>
#include <limits>



class HelloTriangleApplication {
 public:


  void run() {
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  Window window;
  VulkanInstance instance;
  VulkanSurface surface{ instance };
  VulkanPhysicalDevice physicalDevice;
  VulkanLogicalDevice logicalDevice;


  VkSwapchainKHR swapChain;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;


  

 

 

  std::vector<VkImage> swapChainImages;


  void initVulkan() {
	//instance.vulkanInstanceCreator();

    surface.createSurface(instance,window);

    physicalDevice.pickPhysicalDevice(instance,surface);

    logicalDevice.createLogicalDevice(instance,physicalDevice,surface);

    createSwapChain();

    createImageViews();

    createRenderPass();

    createGraphicsPipeline();

    createFramebuffers();

     createCommandPool();

     createCommandBuffer();

     createSyncObjects();
  }


  void mainLoop() {
    while (!window.shouldClose()) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(logicalDevice.getDevice());
  }

  void cleanup() {

      vkDestroySemaphore(logicalDevice.getDevice(), imageAvailableSemaphore, nullptr);
    
      vkDestroySemaphore(logicalDevice.getDevice(), renderFinishedSemaphore, nullptr);
    
      vkDestroyFence(logicalDevice.getDevice(), inFlightFence, nullptr);

      vkDestroyCommandPool(logicalDevice.getDevice(), commandPool, nullptr);

      for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(logicalDevice.getDevice(), framebuffer, nullptr);
        }

     vkDestroyPipeline(logicalDevice.getDevice(), graphicsPipeline, nullptr);

     vkDestroyPipelineLayout(logicalDevice.getDevice(), pipelineLayout, nullptr);

     vkDestroyRenderPass(logicalDevice.getDevice(), renderPass, nullptr);

    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(logicalDevice.getDevice(), imageView, nullptr);
    }

    

    vkDestroySwapchainKHR(logicalDevice.getDevice(), swapChain, nullptr);





    

   
  }


  void createSyncObjects() 
  {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
 


    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


    if (vkCreateSemaphore(logicalDevice.getDevice(), &semaphoreInfo, nullptr,
                          &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(logicalDevice.getDevice(), &semaphoreInfo, nullptr,
                          &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(logicalDevice.getDevice(), &fenceInfo, nullptr, &inFlightFence) !=
            VK_SUCCESS) {
      throw std::runtime_error("failed to create semaphores!");
    }
  }
  void drawFrame() 
  {
        uint32_t imageIndex;
       vkWaitForFences(logicalDevice.getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
       vkResetFences(logicalDevice.getDevice(), 1, &inFlightFence);
       vkAcquireNextImageKHR(logicalDevice.getDevice(), swapChain, UINT64_MAX,
                             imageAvailableSemaphore, VK_NULL_HANDLE,
                             &imageIndex);


       vkResetCommandBuffer(commandBuffer, 0);
       recordCommandBuffer(commandBuffer, imageIndex);

       VkSubmitInfo submitInfo{};
       submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

       VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
       VkPipelineStageFlags waitStages[] = {
           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
       submitInfo.waitSemaphoreCount = 1;
       submitInfo.pWaitSemaphores = waitSemaphores;
       submitInfo.pWaitDstStageMask = waitStages;
       submitInfo.commandBufferCount = 1;
       submitInfo.pCommandBuffers = &commandBuffer;

       VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
       submitInfo.signalSemaphoreCount = 1;
       submitInfo.pSignalSemaphores = signalSemaphores;

       if (vkQueueSubmit(logicalDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFence) !=
           VK_SUCCESS) {
         throw std::runtime_error("failed to submit draw command buffer!");
       }

       VkPresentInfoKHR presentInfo{};
       presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

       presentInfo.waitSemaphoreCount = 1;
       presentInfo.pWaitSemaphores = signalSemaphores;

       VkSwapchainKHR swapChains[] = {swapChain};
       presentInfo.swapchainCount = 1;
       presentInfo.pSwapchains = swapChains;
       presentInfo.pImageIndices = &imageIndex;
       presentInfo.pResults = nullptr;

       vkQueuePresentKHR(logicalDevice.getPresentQueue(), &presentInfo);

  }



  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

      VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }
    

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }

  }



  void createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(logicalDevice.getDevice(), &allocInfo, &commandBuffer) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  
  }


  void createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = physicalDevice.findQueueFamilies(surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(logicalDevice.getDevice(), &poolInfo, nullptr, &commandPool) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
  }



  void createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      VkImageView attachments[] = {swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(logicalDevice.getDevice(), &framebufferInfo, nullptr,
                              &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }

  }


  void createRenderPass() 
  {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(logicalDevice.getDevice(), &renderPassInfo, nullptr, &renderPass) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
  }
  
static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }


VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice.getDevice(), &createInfo, nullptr, &shaderModule) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

  void createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");
  
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};


    // input assembly setup

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    // vertex shader setup

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;  // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;  // Optional

    // viewport scissor setup

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                 VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();



    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Raster setup 

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
    rasterizer.depthBiasClamp = 0.0f;           // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling setup

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;           // Optional
    multisampling.pSampleMask = nullptr;             // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampling.alphaToOneEnable = VK_FALSE;       // Optional


    // Color blending setup

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ZERO;                                        // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;


    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;  // Optional
    colorBlending.blendConstants[1] = 0.0f;  // Optional
    colorBlending.blendConstants[2] = 0.0f;  // Optional
    colorBlending.blendConstants[3] = 0.0f;  // Optional


    // create pipeline layout

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;             // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;          // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;     // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional

    if (vkCreatePipelineLayout(logicalDevice.getDevice(), &pipelineLayoutInfo, nullptr,
                               &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;  // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(logicalDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &graphicsPipeline) != VK_SUCCESS) {
      throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(logicalDevice.getDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice.getDevice(), vertShaderModule, nullptr);
  }


  void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (int i = 0; i < swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];

      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;

      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(logicalDevice.getDevice(), &createInfo, nullptr,
                            &swapChainImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
      }
    }
  }

  void createSwapChain() {
    SwapChainSupportDetails swapChainSupport =
        physicalDevice.querySwapChainSupport(surface);

    VkSurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface.getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = physicalDevice.findQueueFamilies(surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;      // Optional
      createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logicalDevice.getDevice(), &createInfo, nullptr, &swapChain) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logicalDevice.getDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice.getDevice(), swapChain, &imageCount,
                            swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
  }

  
 

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width !=
        (std::numeric_limits<uint32_t>::max)()) {
      return capabilities.currentExtent;
    } else {
      int width, height;
      glfwGetFramebufferSize(window.getWindow(), &width, &height);

      VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height)};

      actualExtent.width =
          std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                     capabilities.maxImageExtent.width);
      actualExtent.height =
          std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                     capabilities.maxImageExtent.height);

      return actualExtent;
    }
  }

  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }


 
 
};

int main() {
  HelloTriangleApplication app;

  try {
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}