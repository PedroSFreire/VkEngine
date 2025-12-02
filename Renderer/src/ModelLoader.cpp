#include "..\headers\ModelLoader.h"
#include <iostream>

#define FASTGLTF_IMPLEMENTATION
#include "fastgltf/core.hpp"
#include <fastgltf/types.hpp>
#include "fastgltf/tools.hpp"

#define fname "C:/Users/pedro/source/repos/VkEngine/scenes/BoxTextured/glTF/BoxTextured.gltf"


void ModelLoader::loadModel(const std::string filePath) {

	tinyobj::ObjReaderConfig config;
	tinyobj::ObjReader reader;


	if (!reader.ParseFromFile(filePath, config)) {
		std::cerr << reader.Error();
	}

	if (!reader.Warning().empty()) {
		std::cout << reader.Warning();
	}


	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	auto attrib = reader.GetAttrib();

	for (auto shape : reader.GetShapes()) {
		for (auto index : shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos = { attrib.vertices[3 * index.vertex_index + 0],
							attrib.vertices[3 * index.vertex_index + 1],
							attrib.vertices[3 * index.vertex_index + 2] };

			vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index + 0],
								1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

			vertex.color = { 1.0f, 1.0f, 1.0f };


			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
};

void ModelLoader::testFunction()
{
	std::filesystem::path path = fname;
	fastgltf::Parser parser;
	auto data = fastgltf::GltfDataBuffer::FromPath(path);
	if (!data) {
		std::filesystem::path cwd = std::filesystem::current_path();
		std::cout << "Current working directory: " << cwd << std::endl;
		std::cerr << "Failed to load glTF file: " << static_cast<int>(data.error()) << std::endl;
		return;
	}
	auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
	if (!asset) {
		std::cerr << "Failed to parse glTF file: " << static_cast<int>(asset.error()) << std::endl;
		return;
	}

}
