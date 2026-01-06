#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <memory>


#include <optional>
#include <string>
#include <array>
#include <glm/glm.hpp>
#include <stb_image.h>
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"



const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";



enum class Filter {
	Nearest = 0,
	Linear = 1
};

enum class MipmapMode {
	Nearest = 0,
	Linear = 1
};

enum class AddressMode {
	Repeat = 0,
	MirroredRepeat = 1,
	ClampToEdge = 2,
	ClampToBorder = 3
};




enum class LightType : uint32_t {
	Directional = 0,
	Point = 1,
	Spot = 2
};

struct LightAsset {
	LightType type;

	glm::vec3 color;
	float intensity;

	float range;

	float spotInnerCos;

	float spotOuterCos;
};

struct LightGPUData {
	uint32_t type;

	glm::vec3 color;
	float intensity;

	glm::vec3 position;
	float range;

	glm::vec3 direction;
	float spotInnerCos;

	float spotOuterCos;
	int padding0;
};

struct NodeAsset {
	std::string name;
	std::optional<uint32_t> meshIndex;
	std::optional<uint32_t> lightIndex;
	std::optional<uint32_t> cameraIndex;

	int parentIndex = -1;
	std::vector<int> children;

	glm::mat4 transform;


	NodeAsset() = default;
	NodeAsset(const NodeAsset& other) = delete;
	NodeAsset(NodeAsset&& other) noexcept : name(std::move(other.name)),
		meshIndex(std::move(other.meshIndex)),
		lightIndex(std::move(other.lightIndex)),
		cameraIndex(std::move(other.cameraIndex)),
		children(std::move(other.children)),
		transform(std::move(other.transform)) {

	}


};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::uint lightCount;
};

struct pushConstants {
	glm::mat4 transform;
	glm::vec4 colorFactor;

	uint32_t metallicFactor;
	uint32_t roughnessFactor;
	uint32_t emissiveStrenght;
	uint32_t padding0;    


	glm::vec3 emissiveFactor;
	uint32_t padding1;      



};


struct MaterialAsset {

	std::string name;
	uint32_t resourceId;
	//optional textures
	std::optional<uint32_t> colorTexId;
	std::optional<uint32_t> metalRoughTexId;
	std::optional<uint32_t> normalTexId;
	std::optional<uint32_t> emissiveTexId;
	std::optional<uint32_t> occlusionTexId;

	//properties non optional
	glm::vec4 colorFactor;
	uint32_t metallicFactor;
	uint32_t roughnessFactor;
	uint32_t emissiveStrenght;
	glm::vec3 emissiveFactor;
	
	//properties optional
	std::optional<uint32_t> normalScale;
	std::optional<uint32_t> occlusionStrenght;
	
};


struct TextureAsset {
	int imageId;
	int samplerId;
	std::string name;
};


struct SamplerAsset {

	std::string name;
	uint32_t resourceId;

	Filter magFilter = Filter::Linear;
	Filter minFilter = Filter::Linear;
	MipmapMode mipMap = MipmapMode::Linear;
	AddressMode addressU = AddressMode::MirroredRepeat;
	AddressMode addressV = AddressMode::MirroredRepeat;
};


struct SamplerResource {


	VulkanSampler sampler;
	std::string name;
};




struct ImageAsset {
	std::string name;
	uint32_t resourceId = -1;

	uint32_t width;
	uint32_t height;
	uint32_t channels;
	stbi_uc* pixels;
	ImageAsset() = default;


	ImageAsset(const ImageAsset&) = delete;
	ImageAsset(ImageAsset&& other) noexcept 
		:width(other.width),
		height(other.height),
		channels(other.channels),
		pixels(std::move(other.pixels)){ }
		

	
};

struct ImageResource {
	std::string name;
	VulkanImage image;
	VulkanImageView imageView;

	ImageResource() = default;
	ImageResource(const ImageResource&) = delete;
	ImageResource(ImageResource&& other) noexcept
		: name(std::move(other.name)), image(std::move(other.image)),
		imageView(std::move(other.imageView))
	{
	}
};


struct MeshBuffers {
	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;

	MeshBuffers() = default;
	MeshBuffers(const MeshBuffers&) = delete;
	MeshBuffers(MeshBuffers&& other) noexcept
		: vertexBuffer(std::move(other.vertexBuffer)),
		indexBuffer(std::move(other.indexBuffer))
	{
	}
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}


};


struct GeoSurface {
	uint32_t startIndex;
	uint32_t count;
	uint32_t materialIndex;
};

struct MeshAsset {

	std::string name;
	uint32_t resourceId;

	std::vector<GeoSurface> surfaces;
	
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	MeshAsset() = default;
	MeshAsset(const MeshAsset&) = delete;
	MeshAsset(MeshAsset&& other) noexcept
		: name(std::move(other.name)),
		surfaces(std::move(other.surfaces)),
		indices(std::move(other.indices)),
		vertices(std::move(other.vertices))
	{
	}
};

struct MeshResource {
	std::string name;

	MeshBuffers meshBuffers;

	MeshResource() = default;
	MeshResource(const MeshResource&) = delete;
	MeshResource(MeshResource&& other) noexcept
		: name(std::move(other.name)),
		meshBuffers(std::move(other.meshBuffers))
	{
	}
};


struct CPUDrawCallData {
	uint32_t materialDescriptorId = -1;
	glm::mat4 transform;
	MaterialAsset* mat;
};


struct CPUDrawCallInstanceData {
	uint32_t meshResourceId = -1;
	uint32_t meshId = -1;
	std::vector<CPUDrawCallData> cpuDrawCalls;
};




struct DrawCallData {

	VulkanDescriptorSet* descriptorSet;
	MaterialAsset* mat;
	glm::mat4 transform;
};

struct DrawCallBatchData {

	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;
	std::vector<DrawCallData> drawCalls;
};



struct SceneData {

	std::vector<std::shared_ptr<MeshAsset>> meshAssets;
	std::vector<std::shared_ptr<ImageAsset>> imageAssets;
	std::vector<std::shared_ptr<SamplerAsset>> samplerAssets;
	std::vector<std::shared_ptr<TextureAsset>> textures;
	std::vector<std::shared_ptr<MaterialAsset>> materials;
	std::vector<std::shared_ptr<NodeAsset>> nodes;
	std::vector<std::shared_ptr<LightAsset>> lights;
	std::vector<uint32_t> rootNodesIds;
};


struct SceneFramesData {
	std::vector<CPUDrawCallInstanceData> drawInstances;
	std::vector<LightGPUData> frameLightData;
	SceneFramesData(std::vector<CPUDrawCallInstanceData>& instances, std::vector<LightGPUData>& lights)
		: drawInstances(instances), frameLightData(lights) {
	}
	SceneFramesData() = default;
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
};



