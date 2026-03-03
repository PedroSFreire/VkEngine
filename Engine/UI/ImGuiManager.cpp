#include "ImGui.h"
#include "../../Renderer/Renderer/VulkanRenderer.h"




ImGuiManager::ImGuiManager(VulkanRenderer& renderer) {
	init(renderer);
}

ImGuiManager::~ImGuiManager() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

}




void ImGuiManager::init(VulkanRenderer& renderer) {
	VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };



	DescriptorPoolCreateInfo imguiPoolInfo{};
	imguiPoolInfo.poolSizes = pool_sizes;
	imguiPoolInfo.maxSets = 1000;
	imguiPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	
	
	imGuiPool.createDescriptorPool(renderer.getLogicalDevice(), imguiPoolInfo);

	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForVulkan(renderer.getWindow().getWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = renderer.getInstance().getInstance();
	init_info.PhysicalDevice = renderer.getPhysicalDevice().getPhysicalDevice();
	init_info.Device = renderer.getLogicalDevice().getDevice();
	init_info.Queue = renderer.getLogicalDevice().getGraphicsQueue();
	init_info.DescriptorPool = imGuiPool.getDescriptorPool();
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.UseDynamicRendering = false;
	init_info.MSAASamples = renderer.getPhysicalDevice().getMsaaSamples();
	init_info.RenderPass = renderer.getRenderPass().getRenderPass();
	init_info.Subpass = 1;

	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();

}


void ImGuiManager::recordFrame( float deltaTime) {

	static float frameTimes[200] = {};
	static int frameIndex = 0;

	frameTimes[frameIndex] = deltaTime;
	frameIndex = (frameIndex + 1) % 200;


	float fps = 1000.0f / deltaTime;
	/*
	float averageFrameTime = 0;

	for (int i = 0; i < 200; i++)
	{
		averageFrameTime += frameTimes[i];
	}

	averageFrameTime = averageFrameTime / 200.0f;


	float averageFps = 1000.0f / averageFrameTime;
	*/


	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	
	ImGui::ShowMetricsWindow();

	ImGui::Begin("Performance");

	ImGui::Text("Frame time: %.2f ms (%.1f FPS)", deltaTime,fps);

	ImGui::PlotLines("Frame Times",frameTimes,200,frameIndex,nullptr,0.0f,40.0f,ImVec2(0, 80));

	ImGui::End();



	ImGui::Render();

}

void ImGuiManager::recordCommands(VulkanCommandBuffer& commandBuffer) const {
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer.getCommandBuffer());
}

