#pragma once

#include "../../Renderer/Includes/VulkanIncludes.h"

#include <vulkan/vulkan.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


class VulkanRenderer;

class ImGuiManager
{
private:
    //fields
    VulkanDescriptorPool imGuiPool;
	bool selected = false;
    std::string selectePath;

public:
    ImGuiManager() = default;
	ImGuiManager(VulkanRenderer& renderer);
    ~ImGuiManager();
    ImGuiManager(const ImGuiManager&) = delete;

    void init(VulkanRenderer& renderer);

	void recordFrame(float deltaTime);

	void recordCommands(VulkanCommandBuffer& commandBuffer) const;  

    void startUpFrame();

	bool isSelected() const { return selected; }

    const std::string& getSelectedPath() const { return selectePath; }
}; 
