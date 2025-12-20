#pragma once

#include "fastgltf/tools.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/util.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "fastgltf/base64.hpp"

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


class GltfLoader
{
private:

    fastgltf::Asset asset;
    std::vector<std::shared_ptr<MeshAsset>> meshes;
	std::vector<std::shared_ptr<ImageResource>> images;
	std::vector<std::shared_ptr<SamplerResource>> samplers;
    std::vector<std::shared_ptr<TextureResource>> textures;
    std::vector<std::shared_ptr<MaterialResource>> materials;
    std::vector<std::shared_ptr<NodeResource>> nodes;
    std::vector<std::shared_ptr<LightResource>> lights;
	std::vector< DrawCallBatchData> drawCalls;
    std::vector<uint32_t> rootNodesIds;

    //per frame light data
	std::vector<LightGPUData> lightData;
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
public:
    GltfLoader() = default;
    ~GltfLoader() = default;
    GltfLoader(const GltfLoader&) = delete;

	const MeshAsset& getMesh(size_t index) const { return *meshes[index]; }
	const ImageResource& getImage(size_t index) const { return *images[index]; }
	const SamplerResource& getSampler(size_t index) const { return *samplers[index]; }
    const VkDescriptorSetLayout& getLayout() const { return descriptorSets[0].getDescriptorSetLayout(); }
    const VkDescriptorSetLayout& getImageLayout() const { return lightDescriptorSet.getDescriptorSetLayout(); }
    const VkDescriptorSet& getMatDescriptor(int i) { return descriptorSets[i].getDescriptorSet(); }
	const size_t getLightCount() const { return lights.size(); }
    void loadGltf(const char* fname, VulkanRenderer& renderer);

    //iterate through scene graphs and call draw functions
    void recordScene(const VulkanRenderer& renderer) ;
	void drawScene(const VulkanRenderer& renderer, VulkanCommandBuffer& cmdBuff) const;
private:
	//Load functions
    void loadMeshes(VulkanRenderer& renderer);
	void loadImages(VulkanRenderer& renderer, const std::filesystem::path& path);
    void loadSamplers(VulkanRenderer& renderer);
    void loadTextures();
    void loadMaterials();
    void loadNodes();
    void loadLights();

	//Load helpers
    void loadNodesData();
    void loadNodesRelatrions();
    bool loadImageData(VulkanRenderer& renderer, stbi_uc* bytes, uint32_t size, ImageResource& tempTexture);
    bool loadImageData(VulkanRenderer& renderer, const std::string& TEXTURE_PATH, ImageResource& tempImage);
    VkFilter getFilter(fastgltf::Filter gltfFilter);
    VkSamplerMipmapMode getFilterMode(fastgltf::Filter gltfFilter);
    VkSamplerAddressMode getWrap(fastgltf::Wrap wrap);
	void createDescriptorSets( VulkanRenderer& renderer);
    void createDefaultImages(VulkanRenderer& renderer);
    //void createDefaultDescriptors(VulkanRenderer& renderer);

	//Draw utils
    void recordNode(const VulkanRenderer& renderer,const int nodeId, glm::mat4& transforMat);
    void recordMesh(const VulkanRenderer & renderer, const int meshId, glm::mat4& transforMat);
};