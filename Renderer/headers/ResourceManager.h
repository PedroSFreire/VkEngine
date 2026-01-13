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

	//creation functions

	void copyBuffer(const VulkanRenderer& renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const;

	void createMeshResources(const VulkanRenderer& renderer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const;

	void createTexture(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, ImageResource& tex) const;

	void createTexture(const VulkanRenderer& renderer, const ImageAsset& data, ImageResource& tex) const;

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



	//creation functions helpers

	void bufferStagedUpload(const VulkanRenderer& renderer, VulkanBuffer& dstBuffer, const void* bufferData, uint32_t size, uint32_t elementCount) const;

	void createVertexBuffer(const VulkanRenderer& renderer, const std::vector<Vertex>& vertices, VulkanBuffer& vertexBuffer) const;

	void createIndexBuffer(const VulkanRenderer& renderer, const std::vector<uint32_t>& indices, VulkanBuffer& indexBuffer) const;

	void createTextureImage(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, VulkanImage& textureImage) const;

	void createTextureImage(const VulkanRenderer& renderer, const ImageAsset& data, VulkanImage& textureImage)const;


	void createTextureImageHelper(const VulkanRenderer& renderer, stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage) const;

	void createSampler(const VulkanRenderer& renderer, SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV) const;

	void transitionImageLayout(const VulkanRenderer& renderer, VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue)const;

	void copyBufferToImage(const VulkanRenderer& renderer, VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height)const;
};



