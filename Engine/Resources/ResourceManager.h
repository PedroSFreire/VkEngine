#pragma once




#include "../Core/defines.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <span>
#include <array>
#include <string_view>
#include "glm/glm.hpp"

#include "../../Renderer/Includes/VulkanIncludes.h"
class VulkanCommandBuffer;

class  VulkanRenderer;

class DescriptorSet;
class DescriptorPool;
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

	VulkanDescriptorSet& getDescriptor(int id) { return descriptorSets[id]; }
	VulkanDescriptorSet& getLightDescriptor() { return lightDescriptorSet; }
	MeshResource& getMesh(int id) { return *meshes[id]; }

	VulkanDescriptorSet& getEnvCubeDescriptor() { return cubeDescriptorSet; }
	VulkanDescriptorSet& getIrradianceCubeDescriptor() { return irradianceCubeDescriptorSet; }
	VulkanDescriptorSet& getPrefilteredCubeDescriptor() { return prefilteredCubeDescriptorSet; }

	const MeshBuffers& getCubeMesh() const { return cubeMesh; }

	//creation functions

	void copyBuffer(const VulkanRenderer& renderer, VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const;

	void createMeshResources(const VulkanRenderer& renderer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const;

	void createSimpleMeshResources(const VulkanRenderer& renderer, const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, MeshBuffers& meshBuffer) const;

	void createTexture(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, ImageResource& tex, TextureType type) const;

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
	SamplerResource defaultCubeSampler;

	bool lightsCreated = false;


	//envoiremnt maps
	VulkanDescriptorPool cubeDescriptorPool;
	VulkanDescriptorSet cubeDescriptorSet;
	VulkanDescriptorSet irradianceCubeDescriptorSet;
	VulkanDescriptorSet prefilteredCubeDescriptorSet;

	ImageResource equirectangularImage;

	CubeMapResource cubemapImage;
	CubeMapResource irradianceImage;
	CubeMapResource prefilteredImage;



	//should prob not be here but cube for cubemaps
	std::vector<SimpleVertex> cubeVertices = {
		{{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
		{{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}
	};

	std::vector<uint32_t> cubeIndices = {
		0, 1, 2, 2, 3, 0,
		4, 7, 6, 6, 5, 4,
		0, 3, 7, 7, 4, 0,
		1, 5, 6, 6, 2, 1,
		0, 4, 5, 5, 1, 0,
		3, 2, 6, 6, 7, 3
	};
	MeshBuffers cubeMesh;


private:
	uint32_t loadImage(const VulkanRenderer& renderer, const ImageAsset& img);
	uint32_t loadSampler(const VulkanRenderer& renderer, const SamplerAsset& sampler);
	uint32_t loadMesh(const VulkanRenderer& renderer, const MeshAsset& mesh);
    uint32_t createDescriptorSet(const VulkanRenderer& renderer, const MaterialAsset& mat, SceneData& scene);
    void createDefaultImages(const VulkanRenderer& renderer);

	void createEnvironmentMaps(const VulkanRenderer& renderer, const std::string& HDRI_PATH);

	void createCubeImage(const VulkanRenderer& renderer, CubeMapResource& resource, uint32_t textSize, uint32_t mipLevels = 1);
	//creation functions helpers

	void bufferStagedUpload(const VulkanRenderer& renderer, VulkanBuffer& dstBuffer, const void* bufferData, uint32_t size, uint32_t elementCount) const;

	void createVertexBuffer(const VulkanRenderer& renderer, const std::vector<Vertex>& vertices, VulkanBuffer& vertexBuffer) const;

	void createSimpleVertexBuffer(const VulkanRenderer& renderer, const std::vector<SimpleVertex>& vertices, VulkanBuffer& vertexBuffer) const;

	void createIndexBuffer(const VulkanRenderer& renderer, const std::vector<uint32_t>& indices, VulkanBuffer& indexBuffer) const;

	void createTextureImage(const VulkanRenderer& renderer, const std::string& TEXTURE_PATH, VulkanImage& textureImage, TextureType type) const;

	void createTextureImage(const VulkanRenderer& renderer, const ImageAsset& data, VulkanImage& textureImage)const;


	void createTextureImageHelper(const VulkanRenderer& renderer, stbi_uc* pixels, int texWidth, int texHeight, VulkanImage& textureImage, TextureType type) const;

	void createTextureImageHelper(const VulkanRenderer& renderer, float* pixels, int texWidth, int texHeight, VulkanImage& textureImage, TextureType type) const;

	void createSampler(const VulkanRenderer& renderer, SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV) const;

	void createSampler(const VulkanRenderer& renderer, SamplerResource& samplerResource, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipMap, VkSamplerAddressMode addressU, VkSamplerAddressMode adressV, VkSamplerAddressMode adressW) const;

	void transitionImageLayout(const VulkanRenderer& renderer, VulkanImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueue, uint32_t destQueue, uint32_t layercount =1, uint32_t levelCount = 1)const;

	void copyBufferToImage(const VulkanRenderer& renderer, VulkanBuffer& buffer, VulkanImage& image, uint32_t width, uint32_t height)const;
};



