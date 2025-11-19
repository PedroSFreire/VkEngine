#include "..\headers\ModelLoader.h"
#include <iostream>



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