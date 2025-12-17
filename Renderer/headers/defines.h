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
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"



const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";


struct drawCallData {
	uint32_t meshIndex;
	glm::mat4 modelMatrix;
};

struct NodeResource {
	std::string name;
	std::optional<uint32_t> meshIndex;
	std::optional<uint32_t> lightIndex;
	std::optional<uint32_t> cameraIndex;

	int parentIndex = -1;
	std::vector<int> children;

	glm::mat4 transform;


	NodeResource() = default;
	NodeResource(const NodeResource& other) = delete;
	NodeResource(NodeResource&& other) noexcept : name(std::move(other.name)),
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
};

struct pushConstants {
	glm::mat4 transform;
	glm::vec4 colorFactor;
	uint32_t metallicFactor;
	uint32_t roughnessFactor;
	uint32_t emissiveStrenght;
<<<<<<< HEAD
	uint32_t padding0;         // 4 bytes to align next member to 16 bytes
	glm::vec3 emissiveFactor;
	uint32_t padding1;         // 4 bytes to align next member to 16 bytes
=======
	glm::vec3 emissiveFactor;
>>>>>>> ba1e340 (fast GLTF is implemented material data is already in gpu just missing the lights .)
};


struct MaterialResource {

	std::string name;
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


struct TextureResource {
	int imageId;
	int samplerId;
	std::string name;
};
struct SamplerResource {
	VulkanSampler sampler;
	std::string name;
};
struct ImageArrayData {
	uint32_t width;
	uint32_t height;
	uint32_t channels;
	stbi_uc* pixels;
	ImageArrayData(int width_, int height_, int channels_, stbi_uc* pixels_)
		: width(width_), height(height_), channels(channels_), pixels(pixels_) {
	}
		
	ImageArrayData(const ImageArrayData&) = delete;
	ImageArrayData(ImageArrayData&& other) {
		width = other.width;
		height = other.height;
		channels = other.channels;
		pixels = other.pixels;
		
		pixels = nullptr;

	}
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

	std::vector<GeoSurface> surfaces;
	MeshBuffers meshBuffers;

	MeshAsset() = default;
	MeshAsset(const MeshAsset&) = delete;
	MeshAsset(MeshAsset&& other) noexcept
		: name(std::move(other.name)),
		surfaces(std::move(other.surfaces)),
		meshBuffers(std::move(other.meshBuffers))
	{
	}
};



namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}