#pragma once



#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorPool.h"

#include "defines.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <span>
#include <array>
#include <string_view>
#include "glm/glm.hpp"
class VulkanCommandBuffer;

class  VulkanRenderer;
struct SceneData;

class ResourceManager
{
private:

    std::vector<std::shared_ptr<MeshResource>> meshes;
	std::vector<std::shared_ptr<ImageResource>> images;
    std::vector<std::shared_ptr<SamplerResource>> samplers;



    //per frame light data

	VulkanBuffer lightBuffer;
    VulkanDescriptorPool lightdescriptorPool;
	VulkanDescriptorSet lightDescriptorSet;

	//Descriptor sets
    VulkanDescriptorPool descriptorPool;
	std::vector<VulkanDescriptorSet> descriptorSets;

    //Default texture data
    const uint8_t numDefaultTextures = 5;

    ImageResource defaultColorImage;
    ImageResource defaultNormalImage;
    ImageResource defaultMetalRoughImage;
    ImageResource defaultOcclusionImage;
    ImageResource defaultEmissiveImage;

    SamplerResource defaultSampler;

	bool lightsCreated = false;
private:
	uint32_t loadImage(const VulkanRenderer& renderer, const ImageAsset& img);
	uint32_t loadSampler(const VulkanRenderer& renderer, const SamplerAsset& sampler);
	uint32_t loadMesh(const VulkanRenderer& renderer, const MeshAsset& mesh);
    uint32_t createDescriptorSet(const VulkanRenderer& renderer, const MaterialAsset& mat, SceneData& scene);
    void createDefaultImages(const VulkanRenderer& renderer);
public: 
    ResourceManager(const VulkanRenderer& renderer);
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = default;


    void loadScene(const VulkanRenderer& renderer, SceneData& scene);
    void loadLights(const VulkanRenderer& renderer, std::vector<LightGPUData>& lights);
	void updateMaterials(const VulkanRenderer& renderer, SceneData& scene);

	VulkanDescriptorSet& getDescriptor(int id) { return descriptorSets[id]; }
	VulkanDescriptorSet& getLightDescriptor() { return lightDescriptorSet; }
	MeshResource& getMesh(int id) { return *meshes[id]; }
};