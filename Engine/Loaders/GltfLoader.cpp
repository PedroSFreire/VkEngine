#include <iostream>
#include <string.h>
#include <omp.h>

#include <stb_image.h>
#include "GltfLoader.h"

#include <chrono>

//std::chrono::high_resolution_clock timer;
//std::chrono::time_point<std::chrono::high_resolution_clock> start,end;
//std::chrono::duration<double, std::milli> duration;
//start = timer.now();
//end = timer.now();
//duration = end - start;
//std::cout << "Loading to structs " << duration.count() << " ms\n";

namespace MikkTSpaceCallbacks {
	int getNumFaces(const SMikkTSpaceContext* ctx) {
		auto* mesh = (MeshAsset*)ctx->m_pUserData;
		return mesh->indices.size() / 3;
	}


	int getNumVerticesOfFace(const SMikkTSpaceContext*, int) {
		return 3;
	}


	void getPosition(
		const SMikkTSpaceContext* ctx,
		float pos[3],
		int face,
		int vert)
	{
		auto* mesh = (MeshAsset*)ctx->m_pUserData;
		uint32_t index = mesh->indices[face * 3 + vert];
		const glm::vec3& p = mesh->vertices[index].pos;

		pos[0] = p.x;
		pos[1] = p.y;
		pos[2] = p.z;
	}


	void getNormal(
		const SMikkTSpaceContext* ctx,
		float normal[3],
		int face,
		int vert)
	{
		auto* mesh = (MeshAsset*)ctx->m_pUserData;
		uint32_t index = mesh->indices[face * 3 + vert];
		const glm::vec3& n = mesh->vertices[index].normal;

		normal[0] = n.x;
		normal[1] = n.y;
		normal[2] = n.z;
	}


	void getTexCoord(
		const SMikkTSpaceContext* ctx,
		float uv[2],
		int face,
		int vert)
	{
		auto* mesh = (MeshAsset*)ctx->m_pUserData;
		uint32_t index = mesh->indices[face * 3 + vert];
		const glm::vec2& t = mesh->vertices[index].texCoord;

		uv[0] = t.x;
		uv[1] = t.y;
	}


	void setTSpaceBasic(const SMikkTSpaceContext* ctx, const float tangent[3], float sign, int face, int vert)
	{
		auto* mesh = (MeshAsset*)ctx->m_pUserData;
		uint32_t index = mesh->indices[face * 3 + vert];

		mesh->vertices[index].tangent = glm::vec4(
			tangent[0],
			tangent[1],
			tangent[2],
			sign
		);
	}
}

//Loading helper functions

AddressMode GltfLoader::getWrap(fastgltf::Wrap wrap) {
	AddressMode wrapMode = AddressMode::MirroredRepeat;
	switch (wrap) {
	case fastgltf::Wrap::ClampToEdge:
		wrapMode = AddressMode::ClampToEdge;
		break;
	case fastgltf::Wrap::MirroredRepeat:
		wrapMode = AddressMode::MirroredRepeat;
		break;
	case fastgltf::Wrap::Repeat:
		wrapMode = AddressMode::Repeat;
		break;
	default:
		break;
	}
	return wrapMode;
}

Filter GltfLoader::getFilter(fastgltf::Filter gltfFilter) {
	Filter filter = Filter::Linear;
	switch (gltfFilter) {
	case fastgltf::Filter::Nearest:
	case fastgltf::Filter::NearestMipMapNearest:
	case fastgltf::Filter::NearestMipMapLinear:
		filter = Filter::Nearest;
		break;
	case fastgltf::Filter::LinearMipMapNearest:
	case fastgltf::Filter::LinearMipMapLinear:
	case fastgltf::Filter::Linear:
		filter = Filter::Linear;
		break;
	default:
		break;
	}
	return filter;

}

MipmapMode GltfLoader::getFilterMode(fastgltf::Filter gltfFilter) {
	MipmapMode mipMap = MipmapMode::Linear;
	switch (gltfFilter) {
	case fastgltf::Filter::NearestMipMapNearest:
	case fastgltf::Filter::LinearMipMapNearest:
		mipMap = MipmapMode::Nearest;
		break;

	case fastgltf::Filter::NearestMipMapLinear:
	case fastgltf::Filter::LinearMipMapLinear:
		mipMap = MipmapMode::Linear;
		break;
	default:
		break;

	}
	return mipMap;
}

LightType GltfLoader::getLightType(fastgltf::LightType gltfType){
	LightType lType = LightType::Point;
	switch (gltfType) {

	case fastgltf::LightType::Directional:
		lType = LightType::Directional;
		break;

	case fastgltf::LightType::Point:
		lType = LightType::Point;
		break;

	case fastgltf::LightType::Spot:
		lType = LightType::Spot;
		break;
	default:
		break;

	}
	return lType;
}


//new scene loading helpers

void GltfLoader::loadNodesData(SceneData& scene) {

	
	scene.nodes.reserve(asset.nodes.size());
	for (auto& node : asset.nodes) {
		NodeAsset newNode;

		newNode.name = node.name;

		if (node.meshIndex.has_value())
			newNode.meshIndex = node.meshIndex.value();
		if (node.cameraIndex.has_value())
			newNode.cameraIndex = node.cameraIndex.value();
		if (node.lightIndex.has_value())
			newNode.lightIndex = node.lightIndex.value();



		if (auto transform = std::get_if<fastgltf::TRS>(&node.transform)) {
			newNode.transform = glm::translate(glm::mat4(1.0f), glm::vec3(transform->translation[0], transform->translation[1], transform->translation[2]));
			glm::quat glmQuat(transform->rotation.w(), transform->rotation.x(), transform->rotation.y(), transform->rotation.z());
			newNode.transform *= glm::mat4_cast(glmQuat);
			newNode.transform = glm::scale(newNode.transform, glm::vec3(transform->scale[0], transform->scale[1], transform->scale[2]));
		}
		else if (auto transform = std::get_if<fastgltf::math::fmat4x4>(&node.transform)) {
			for (int row = 0; row < 4; ++row)
				for (int col = 0; col < 4; ++col)
					newNode.transform[col][row] = transform->data()[row];
		}
		scene.nodes.emplace_back(std::make_shared<NodeAsset>(std::move(newNode)));
	}
}

void GltfLoader::loadNodesRelatrions(SceneData& scene) {
	for (int nodeId = 0; nodeId < asset.nodes.size(); nodeId++) {
		auto& node = asset.nodes[nodeId];
		auto& NodeAsset = scene.nodes[nodeId];
		if (node.children.size() > 0) {
			NodeAsset->children.reserve(node.children.size());
			for (auto& childId : node.children) {
				NodeAsset->children.emplace_back(childId);
				scene.nodes[childId].get()->parentIndex = nodeId;

			}
		}

	}

	for (int nodeId = 0; nodeId < asset.nodes.size(); nodeId++) {
		if (scene.nodes[nodeId]->parentIndex == -1) {
			scene.rootNodesIds.emplace_back(nodeId);
		}

	}


}

bool GltfLoader::loadImageData(SceneData& scene, stbi_uc* bytes, uint32_t size, ImageAsset& tempImage, int id) {
	int width, height, channels;

	stbi_uc* pixels = stbi_load_from_memory(bytes, size, &width, &height, &channels, STBI_rgb_alpha);



	if (!pixels || width == 0 || height == 0 || channels == 0) {
		return false;
	}

	ImageAsset imgArray;
	imgArray.height = height;
	imgArray.width = width;
	imgArray.channels = channels;
	imgArray.pixels = pixels;
#pragma omp critical
		scene.imageAssets[id] = (std::make_shared<ImageAsset>(std::move(imgArray)));

	return true;
}

bool GltfLoader::loadImageData(SceneData& scene, const std::string& TEXTURE_PATH, ImageAsset& tempImage, int id) {
	int width, height, channels;

	stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &width, &height, &channels, STBI_rgb_alpha);




	if (!pixels || width == 0 || height == 0 || channels == 0) {
		return false;
	}

	ImageAsset imgArray;
	imgArray.height = height;
	imgArray.width = width;
	imgArray.channels = channels;
	imgArray.pixels = pixels;
#pragma omp critical
		scene.imageAssets[id] = (std::make_shared<ImageAsset>(std::move(imgArray)));
	return true;
}



//load scene functions

void GltfLoader::loadNodes(SceneData& scene) {
	loadNodesData(scene);
	loadNodesRelatrions(scene);
}

void GltfLoader::loadLights(SceneData& scene) {
	scene.lights.reserve(asset.lights.size());
	for (auto& light : asset.lights) {
		LightAsset newLight;
		newLight.type = getLightType(light.type);
		newLight.color.x = light.color[0];
		newLight.color.y = light.color[1];
		newLight.color.z = light.color[2];
		newLight.intensity = light.intensity;
		//point and spot lights

		if (light.range.has_value())
			newLight.range = light.range.value();
		else
			newLight.range = 100.0f;

		if (light.innerConeAngle.has_value())
			newLight.spotInnerCos = glm::cos(light.innerConeAngle.value());
		if (light.outerConeAngle.has_value())
			newLight.spotOuterCos = glm::cos(light.outerConeAngle.value());
		scene.lights.emplace_back(std::make_shared<LightAsset>(std::move(newLight)));
	}


}

void GltfLoader::loadMaterials(SceneData& scene) {
	/*struct MaterialAsset {

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


	};*/
	scene.materials.reserve(asset.materials.size());

	for (auto& material : asset.materials) {

		MaterialAsset newMaterial;
		newMaterial.name = material.name;
		//non optional properties
		newMaterial.colorFactor.x = material.pbrData.baseColorFactor[0];
		newMaterial.colorFactor.y = material.pbrData.baseColorFactor[1];
		newMaterial.colorFactor.z = material.pbrData.baseColorFactor[2];
		newMaterial.colorFactor.w = material.pbrData.baseColorFactor[3];

		newMaterial.metallicFactor = material.pbrData.metallicFactor;
		newMaterial.roughnessFactor = material.pbrData.roughnessFactor;
		newMaterial.emissiveStrenght = material.emissiveStrength;

		newMaterial.emissiveFactor.x = material.emissiveFactor[0];
		newMaterial.emissiveFactor.y = material.emissiveFactor[1];
		newMaterial.emissiveFactor.z = material.emissiveFactor[2];

		//texture ids all are optional
		if (material.pbrData.baseColorTexture.has_value()) {
			newMaterial.colorTexId = material.pbrData.baseColorTexture.value().textureIndex;
			int imageId = scene.textures[newMaterial.colorTexId.value()]->imageId;
			scene.imageAssets[imageId]->type = TextureType::Color;
		}

		if (material.pbrData.metallicRoughnessTexture.has_value()) {
			newMaterial.metalRoughTexId = material.pbrData.metallicRoughnessTexture.value().textureIndex;
			int imageId = scene.textures[newMaterial.metalRoughTexId.value()]->imageId;
			scene.imageAssets[imageId]->type = TextureType::MetallicRoughnessAO;
		}

		if (material.normalTexture.has_value()) {
			newMaterial.normalTexId = material.normalTexture.value().textureIndex;
			newMaterial.normalScale = material.normalTexture.value().scale;
			int imageId = scene.textures[newMaterial.normalTexId.value()]->imageId;
			scene.imageAssets[imageId]->type = TextureType::Normal;

		}

		if (material.occlusionTexture.has_value()) {
			newMaterial.occlusionTexId = material.occlusionTexture.value().textureIndex;
			newMaterial.occlusionTexId = material.occlusionTexture.value().strength;
			int imageId = scene.textures[newMaterial.occlusionTexId.value()]->imageId;
			scene.imageAssets[imageId]->type = TextureType::Occlusion;
		}

		if (material.emissiveTexture.has_value()) {
			newMaterial.emissiveTexId = material.emissiveTexture.value().textureIndex;
			int imageId = scene.textures[newMaterial.emissiveTexId.value()]->imageId;
			scene.imageAssets[imageId]->type = TextureType::Emissive;
		}
		scene.materials.emplace_back(std::make_shared<MaterialAsset>(std::move(newMaterial)));
	}
}

void GltfLoader::loadSamplers(SceneData& scene) {

	scene.samplerAssets.reserve(asset.samplers.size());
	for (auto& sampler : asset.samplers) {
		SamplerResource newSampler;
		SamplerAsset newSamplerAsset;



		newSamplerAsset.addressU = getWrap(sampler.wrapS);
		newSamplerAsset.addressV = getWrap(sampler.wrapT);


		//interpret mipMap
		if (sampler.magFilter.has_value()) {
			newSamplerAsset.mipMap = getFilterMode(sampler.magFilter.value());
		}

		//interpret magFilter
		if (sampler.magFilter.has_value()) {
			newSamplerAsset.magFilter = getFilter(sampler.magFilter.value());
		}
		//interpret minFilter
		if (sampler.minFilter.has_value()) {
			newSamplerAsset.magFilter = getFilter(sampler.minFilter.value());
		}

		scene.samplerAssets.emplace_back(std::make_shared<SamplerAsset>(std::move(newSamplerAsset)));
	}

	//renderer.createSampler(defaultSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

void GltfLoader::loadTextures(SceneData& scene) {
	scene.textures.reserve(asset.textures.size());
	for (auto& texture : asset.textures) {
		TextureAsset newTexture;
		newTexture.name = texture.name;
		if (!texture.imageIndex.has_value() || !texture.samplerIndex.has_value())
			std::cout << "Texture missing image or sampler index\n";
		newTexture.imageId = texture.imageIndex.value();
		newTexture.samplerId = texture.samplerIndex.value();
		scene.textures.emplace_back(std::make_shared<TextureAsset>(std::move(newTexture)));
	}
}

void GltfLoader::loadImages(SceneData& scene, const std::filesystem::path& path)
{
	
	scene.imageAssets.resize(asset.images.size());

#pragma omp parallel for	
	for (int imgId = 0; imgId < asset.images.size(); imgId++) {

		auto& image = asset.images[imgId];
		ImageAsset tempImage;
		tempImage.name = image.name;

		std::string imagePath;

		//Load Based on variant type
		if (auto filename = std::get_if<fastgltf::sources::URI>(&image.data)) {
			//type URI just a file path
			imagePath = path.parent_path().string() + "/" + filename->uri.c_str();

			if (!loadImageData(scene, imagePath, tempImage, imgId)) {
				continue;
			}

		}
		else if (fastgltf::sources::Array* imageArray = std::get_if<fastgltf::sources::Array>(&image.data)) {
			//type array an arry of bytes of data
			stbi_uc* bytes = reinterpret_cast<stbi_uc*>(imageArray->bytes.data());
			int width, height, channels;

			if (!loadImageData(scene, bytes, imageArray->bytes.size(),tempImage, imgId)) {
				continue;
			}

		}
		else if (auto bufferViewId = std::get_if<fastgltf::sources::BufferView>(&image.data)) {
			//type bufferView

			auto bufferView = &(asset.bufferViews[bufferViewId->bufferViewIndex]);
			auto& buffer = asset.buffers[bufferView->bufferIndex];
			auto bufferAdress = std::get_if<fastgltf::sources::Array>(&buffer.data)->bytes.data();
			auto data = reinterpret_cast<stbi_uc*>(bufferAdress + bufferView->byteOffset);
			auto size = bufferView->byteLength;

			if (!loadImageData(scene, data, size, tempImage, imgId)) {
				continue;
			}

		}
		


	}

}

void GltfLoader::loadMeshes(SceneData& scene)
{

	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	scene.meshAssets.reserve(asset.meshes.size()+ scene.meshAssets.size());

	for (const auto& mesh : asset.meshes) {


		MeshAsset newMeshAsset;

		newMeshAsset.name = mesh.name;


		indices.clear();
		vertices.clear();

		//allocation for drawCall struct


		for (const auto& primitive : mesh.primitives) {
			GeoSurface newSurface;
			if (primitive.materialIndex.has_value())
				newSurface.materialIndex = primitive.materialIndex.value();
			newSurface.startIndex = (uint32_t)indices.size();
			newSurface.count = (uint32_t)asset.accessors[primitive.indicesAccessor.value()].count;

			size_t initial_vtx = vertices.size();


			// load indexes

			fastgltf::Accessor& indexaccessor = asset.accessors[primitive.indicesAccessor.value()];
			indices.reserve(indices.size() + indexaccessor.count);

			fastgltf::iterateAccessor<std::uint32_t>(asset, indexaccessor,
				[&](std::uint32_t idx) {
					indices.push_back(idx + initial_vtx);
				});
			// load vertex positions
			{

				fastgltf::Accessor& posAccessor = asset.accessors[primitive.findAttribute("POSITION")->accessorIndex];
				vertices.resize(vertices.size() + posAccessor.count);

				fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, posAccessor,
					[&](glm::vec3 v, size_t index) {
						Vertex newvtx;
						newvtx.pos = v;
						newvtx.normal = { 1, 0, 0 };
						newvtx.color = glm::vec4{ 1.f };
						newvtx.texCoord = { 0,0 };
						vertices[initial_vtx + index] = newvtx;
					});
			}

			// load vertex normals
			auto normals = primitive.findAttribute("NORMAL");
			if (normals != primitive.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, asset.accessors[(*normals).accessorIndex],
					[&](glm::vec3 v, size_t index) {
						vertices[initial_vtx + index].normal = v;
					});
			}

			// load UVs
			auto uv = primitive.findAttribute("TEXCOORD_0");
			if (uv != primitive.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, asset.accessors[(*uv).accessorIndex],
					[&](glm::vec2 v, size_t index) {
						vertices[initial_vtx + index].texCoord = v;
					});
			}

			// load vertex colors
			auto colors = primitive.findAttribute("COLOR_0");
			if (colors != primitive.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec4>(asset, asset.accessors[(*colors).accessorIndex],
					[&](glm::vec4 v, size_t index) {
						vertices[initial_vtx + index].color = v;
					});
			}
			newMeshAsset.surfaces.push_back(newSurface);


		}
		newMeshAsset.vertices = std::move(vertices);
		newMeshAsset.indices = std::move(indices);

		ctx.m_pUserData = &newMeshAsset;

		genTangSpaceDefault(&ctx);

		scene.meshAssets.emplace_back(std::make_shared<MeshAsset>(std::move(newMeshAsset)));
	}
}

void GltfLoader::loadGltf(SceneData& scene,const char* fname)
{
	std::filesystem::path path = fname;

	constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
		| fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages;

	constexpr fastgltf::Extensions gltfExtensions = fastgltf::Extensions::KHR_lights_punctual | fastgltf::Extensions::KHR_texture_basisu | fastgltf::Extensions::KHR_materials_ior | fastgltf::Extensions::KHR_materials_specular | fastgltf::Extensions::KHR_materials_transmission | fastgltf::Extensions::KHR_materials_emissive_strength;

	fastgltf::Parser parser(gltfExtensions);
	auto data = fastgltf::GltfDataBuffer::FromPath(path);
	if (!data) {
		std::filesystem::path cwd = std::filesystem::current_path();
		std::cout << "Current working directory: " << cwd << std::endl;
		std::cerr << "Failed to load glTF file: " << static_cast<int>(data.error()) << std::endl;
		return;
	}
	auto assetOptional = parser.loadGltf(data.get(), path.parent_path(), gltfOptions);
	if (!assetOptional) {
		std::cerr << "Failed to parse glTF file: " << static_cast<int>(assetOptional.error()) << std::endl;
		return;
	}
	asset = std::move(assetOptional.get());

	SMikkTSpaceInterface iface = {};
	iface.m_getNumFaces = MikkTSpaceCallbacks::getNumFaces;
	iface.m_getNumVerticesOfFace = MikkTSpaceCallbacks::getNumVerticesOfFace;
	iface.m_getPosition = MikkTSpaceCallbacks::getPosition;
	iface.m_getNormal = MikkTSpaceCallbacks::getNormal;
	iface.m_getTexCoord = MikkTSpaceCallbacks::getTexCoord;
	iface.m_setTSpaceBasic = MikkTSpaceCallbacks::setTSpaceBasic;

	ctx.m_pInterface = &iface;


	loadImages(scene, path);
	loadSamplers(scene);
	loadTextures(scene);
	loadMeshes(scene);
	loadMaterials(scene);
	loadNodes(scene);
	loadLights(scene);



}











