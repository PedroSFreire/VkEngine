#include "..\headers\GltfLoader.h"
#include <iostream>
#include <string.h>
#include <omp.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



#include "..\headers\VulkanRenderer.h"
#include "..\headers\VulkanSampler.h"
#include "..\headers\VulkanCommandBuffer.h"

#include <stb_image.h>


#include <chrono>
//std::chrono::high_resolution_clock timer;
//std::chrono::time_point<std::chrono::high_resolution_clock> start,end;
//std::chrono::duration<double, std::milli> duration;
//start = timer.now();
//end = timer.now();
//duration = end - start;
//std::cout << "Loading to structs " << duration.count() << " ms\n";



//Loading helper functions
VkSamplerAddressMode GltfLoader::getWrap(fastgltf::Wrap wrap) {
	VkSamplerAddressMode wrapMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	switch (wrap) {
		case fastgltf::Wrap::ClampToEdge:
			wrapMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case fastgltf::Wrap::MirroredRepeat:
			wrapMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		case fastgltf::Wrap::Repeat:
			wrapMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		default:
			break;
	}
	return wrapMode;
}

VkFilter GltfLoader::getFilter(fastgltf::Filter gltfFilter) {
	VkFilter filter = VK_FILTER_LINEAR;
	switch (gltfFilter) {
		case fastgltf::Filter::Nearest:
		case fastgltf::Filter::NearestMipMapNearest:
		case fastgltf::Filter::NearestMipMapLinear:
			filter = VK_FILTER_NEAREST;
			break;
		case fastgltf::Filter::LinearMipMapNearest:
		case fastgltf::Filter::LinearMipMapLinear:
		case fastgltf::Filter::Linear:
			filter = VK_FILTER_LINEAR;
			break;
		default:
			break;
	}
	return filter;
	
}

VkSamplerMipmapMode GltfLoader::getFilterMode(fastgltf::Filter gltfFilter) {
	VkSamplerMipmapMode mipMap = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	switch (gltfFilter) {
	case fastgltf::Filter::NearestMipMapNearest:
	case fastgltf::Filter::LinearMipMapNearest:
		mipMap = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;

	case fastgltf::Filter::NearestMipMapLinear:
	case fastgltf::Filter::LinearMipMapLinear:
		mipMap = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	default:
		break;

	}
	return mipMap;
}

bool GltfLoader::loadImageData(VulkanRenderer& renderer, stbi_uc* bytes, uint32_t size, ImageResource& tempImage) {
	int width, height, channels;

	stbi_uc* pixels = stbi_load_from_memory(bytes, size, &width, &height, &channels, STBI_rgb_alpha);



	if (!pixels || width == 0 || height == 0 || channels == 0) {
		return false;
	}

	ImageArrayData imgArray(width, height, channels, pixels);
#pragma omp critical
	renderer.createTexture(imgArray, tempImage);
	return true;
}

bool GltfLoader::loadImageData(VulkanRenderer& renderer, const std::string& TEXTURE_PATH, ImageResource& tempImage) {
	int width, height, channels;

	stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	



	if (!pixels || width == 0 || height == 0 || channels == 0) {
		return false;
	}

	ImageArrayData imgArray(width, height, channels, pixels);
#pragma omp critical
	renderer.createTexture(imgArray, tempImage);
	return true;
}

void GltfLoader::loadNodesData() {

	nodes.reserve(asset.nodes.size());
	for (auto& node : asset.nodes) {
		NodeResource newNode;

		newNode.name = node.name;

		if(node.meshIndex.has_value())
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
		else if(auto transform = std::get_if<fastgltf::math::fmat4x4>(&node.transform)){
			for (int row = 0; row < 4; ++row)
				for (int col = 0; col < 4; ++col)
					newNode.transform[col][row] = transform->data()[row];
		}
		nodes.emplace_back(std::make_shared<NodeResource>(std::move(newNode)));	
	}
}

void GltfLoader::loadNodesRelatrions(){
	for (int nodeId = 0; nodeId < asset.nodes.size();nodeId++) {
		auto& node = asset.nodes[nodeId];
		auto& nodeResource = nodes[nodeId];
		if (node.children.size() > 0) {
			nodeResource->children.reserve(node.children.size());
			for (auto& childId : node.children) {
				nodeResource->children.emplace_back(childId);
				nodes[childId].get()->parentIndex = nodeId;

			}
		}

	}

	for (int nodeId = 0; nodeId < asset.nodes.size(); nodeId++) {
		if (nodes[nodeId]->parentIndex == -1) {
			rootNodesIds.emplace_back(nodeId);
		}

	}


}

void GltfLoader::createDescriptorSets(VulkanRenderer& renderer) {
	//TO DO
	descriptorPool.createMaterialDescriptorPool(renderer.getLogicalDevice(), materials.size());
	descriptorSets.reserve(materials.size());

	for (auto& materialP : materials) {
		VulkanDescriptorSet newSet;
		newSet.createMaterialDescriptorLayout(renderer.getLogicalDevice(), descriptorPool);
		newSet.createDescriptor();
		//TODO:load default textures
		ImageResource* colorImage;
		SamplerResource* colorSampler;

		ImageResource* normalImage;
		SamplerResource* normalSampler;

		ImageResource* metalRoughImage;
		SamplerResource* metalRoughSampler;

		ImageResource* occlusionImage;
		SamplerResource* occlusionSampler;

		ImageResource* emissiveImage;
		SamplerResource* emissiveSampler;

		//load color texture
		if (materialP->colorTexId.has_value()) {
			auto& colorTexture = textures[materialP->colorTexId.value()];
			colorImage = images[colorTexture->imageId].get();
			colorSampler = samplers[colorTexture->samplerId].get();

		}
		else {
			colorImage = &defaultColorImage;
			colorSampler = &defaultSampler;
		}

		//load nomral texture
		if (materialP->normalTexId.has_value()) {
			auto& normalTexture = textures[materialP->normalTexId.value()];
			normalImage = images[normalTexture->imageId].get();
			normalSampler = samplers[normalTexture->samplerId].get();

		}
		else {
			normalImage = &defaultNormalImage;
			normalSampler = &defaultSampler;
		}

		//load metalRough texture
		if (materialP->metalRoughTexId.has_value()) {
			auto& metalRoughTexture = textures[materialP->metalRoughTexId.value()];
			metalRoughImage = images[metalRoughTexture->imageId].get();
			metalRoughSampler = samplers[metalRoughTexture->samplerId].get();

		}
		else {
			metalRoughImage = &defaultMetalRoughImage;
			metalRoughSampler = &defaultSampler;
		}

		//load occlusion texture
		if (materialP->occlusionTexId.has_value()) {
			auto& occlusionTexture = textures[materialP->occlusionTexId.value()];
			occlusionImage = images[occlusionTexture->imageId].get();
			occlusionSampler = samplers[occlusionTexture->samplerId].get();

		}
		else {
			occlusionImage = &defaultOcclusionImage;
			occlusionSampler = &defaultSampler;
		}

		//load emissive texture
		if (materialP->emissiveTexId.has_value()) {
			auto& emissiveTexture = textures[materialP->emissiveTexId.value()];
			emissiveImage = images[emissiveTexture->imageId].get();
			emissiveSampler = samplers[emissiveTexture->samplerId].get();

		}
		else {
			emissiveImage = &defaultEmissiveImage;
			emissiveSampler = &defaultSampler;
		}

		std::array<VkImageView,5> imageViews{colorImage->imageView.getImageView(),normalImage->imageView.getImageView(),metalRoughImage->imageView.getImageView(),occlusionImage->imageView.getImageView(),emissiveImage->imageView.getImageView() };
		std::array<VkSampler, 5> imageSamplers{colorSampler->sampler.getSampler(),normalSampler->sampler.getSampler(),metalRoughSampler->sampler.getSampler(),occlusionSampler->sampler.getSampler(),emissiveSampler->sampler.getSampler()};
		newSet.updateMaterialDescriptor(imageViews.data(), imageSamplers.data());
		descriptorSets.emplace_back(std::move(newSet));

		
	}
	lightdescriptorPool.createLightDescriptorPool(renderer.getLogicalDevice(), 1);
	lightDescriptorSet.createLightDescriptorLayout(renderer.getLogicalDevice(), lightdescriptorPool);
	lightDescriptorSet.createDescriptor();

}

void GltfLoader::createDefaultImages(VulkanRenderer& renderer) {


	//create color default tex
	std::array<uint8_t, 4> colorPixel{ 255, 255, 255, 255 };
	defaultColorImage.name = "color";
	ImageArrayData imgData{ 1,1,4,colorPixel.data() };
	renderer.createTexture(imgData, defaultColorImage);

	//create normal default tex
	std::array<uint8_t, 4> normalPixel{ 255, 255, 255, 255 };
	defaultNormalImage.name = "normal";
	ImageArrayData normalData{ 1,1,4,normalPixel.data() };
	renderer.createTexture(normalData, defaultNormalImage);

	//create normal metal rough tex
	std::array<uint8_t, 4> metalRoughPixel{ 255, 255, 255, 255 };
	defaultMetalRoughImage.name = "metalRough";
	ImageArrayData metalRoughData{ 1,1,4,metalRoughPixel.data() };
	renderer.createTexture(metalRoughData, defaultMetalRoughImage);

	//create occlusion default tex
	std::array<uint8_t, 4> occlusionPixel{ 255, 255, 255, 255 };
	defaultOcclusionImage.name = "occlusion";
	ImageArrayData occlusionData{ 1,1,4,occlusionPixel.data() };
	renderer.createTexture(occlusionData, defaultOcclusionImage);

	//create emissive default tex
	std::array<uint8_t, 4> emissivePixel{ 255, 255, 255, 255 };
	defaultEmissiveImage.name = "emissive";
	ImageArrayData emissiveData{ 1,1,4,emissivePixel.data() };
	renderer.createTexture(emissiveData, defaultEmissiveImage);

}




//Loading functions
void GltfLoader::loadNodes() {
	loadNodesData();
	loadNodesRelatrions();
}

void GltfLoader::loadLights() {
	lights.reserve(asset.lights.size());
	for (auto& light : asset.lights) {
		LightResource newLight;
		newLight.type = static_cast<uint32_t>(light.type);
		newLight.color.x = light.color[0];
		newLight.color.y = light.color[1];
		newLight.color.z = light.color[2];
		newLight.intensity = light.intensity;
		//point and spot lights

		if(light.range.has_value())
			newLight.range = light.range.value();
		else
			newLight.range = 100.0f;

		if (light.innerConeAngle.has_value())
			newLight.spotInnerCos = light.innerConeAngle.value();
		if (light.outerConeAngle.has_value())
			newLight.spotOuterCos = light.outerConeAngle.value();
		lights.emplace_back(std::make_shared<LightResource>(std::move(newLight)));
	}
}

void GltfLoader::loadMaterials() {
	/*struct MaterialResource {

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
	materials.reserve(asset.materials.size());
	int i = 0;
	for (auto& material : asset.materials) {

		MaterialResource newMaterial;
		newMaterial.name = material.name;
		//non optional properties
		newMaterial.colorFactor.x = material.pbrData.baseColorFactor[0];
		newMaterial.colorFactor.y = material.pbrData.baseColorFactor[1];
		newMaterial.colorFactor.z = material.pbrData.baseColorFactor[2];
		newMaterial.colorFactor.w = material.pbrData.baseColorFactor[3];
		if( i == 19 || i== 20)
			std::cout << "red " << std::endl;
		if (newMaterial.colorFactor == glm::vec4(1, 0, 0,1))
			std::cout << "red " << std::endl;
		newMaterial.metallicFactor = material.pbrData.metallicFactor;
		newMaterial.roughnessFactor = material.pbrData.roughnessFactor;
		newMaterial.emissiveStrenght = material.emissiveStrength;

		newMaterial.emissiveFactor.x = material.emissiveFactor[0];
		newMaterial.emissiveFactor.y = material.emissiveFactor[1];
		newMaterial.emissiveFactor.z = material.emissiveFactor[2];

		//texture ids all are optional
		if (material.pbrData.baseColorTexture.has_value()) {
			newMaterial.colorTexId = material.pbrData.baseColorTexture.value().textureIndex;
		}

		if (material.pbrData.metallicRoughnessTexture.has_value()){
			newMaterial.metalRoughTexId = material.pbrData.metallicRoughnessTexture.value().textureIndex;
		}

		if (material.normalTexture.has_value()){
			newMaterial.normalTexId = material.normalTexture.value().textureIndex;
			newMaterial.normalScale = material.normalTexture.value().scale;
		}

		if (material.occlusionTexture.has_value()){
			newMaterial.occlusionTexId = material.occlusionTexture.value().textureIndex;
			newMaterial.occlusionTexId = material.occlusionTexture.value().strength;
		}

		if (material.emissiveTexture.has_value()){
			newMaterial.emissiveTexId = material.emissiveTexture.value().textureIndex;
		}
		materials.emplace_back(std::make_shared<MaterialResource>(std::move(newMaterial)));
		i++;
	}
}

void GltfLoader::loadSamplers(VulkanRenderer& renderer) {
	samplers.reserve(asset.samplers.size());
	for(auto& sampler : asset.samplers) {
		SamplerResource newSampler;
		newSampler.name = sampler.name;
		VkFilter magFilter = VK_FILTER_LINEAR;
		VkFilter minFilter = VK_FILTER_LINEAR;
		VkSamplerMipmapMode mipMap = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode addressU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		addressU = getWrap(sampler.wrapS);
		addressV = getWrap(sampler.wrapT);


		//interpret mipMap
		if (sampler.magFilter.has_value()) {
			mipMap = getFilterMode(sampler.magFilter.value());
		}
		
		//interpret magFilter
		if (sampler.magFilter.has_value()) {
			magFilter = getFilter(sampler.magFilter.value());
		}
		//interpret minFilter
		if (sampler.minFilter.has_value()) {
			minFilter = getFilter(sampler.minFilter.value());
		}


		renderer.createSampler(newSampler,magFilter,minFilter,mipMap,addressU,addressV);
		samplers.emplace_back(std::make_shared<SamplerResource>(std::move(newSampler)));
	}

	renderer.createSampler(defaultSampler,VK_FILTER_LINEAR,VK_FILTER_LINEAR,VK_SAMPLER_MIPMAP_MODE_LINEAR,VK_SAMPLER_ADDRESS_MODE_REPEAT,VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

void GltfLoader::loadTextures() {
	for (auto& texture : asset.textures) {
		TextureResource newTexture;
		newTexture.name = texture.name;
		if (!texture.imageIndex.has_value() || !texture.samplerIndex.has_value())
			std::cout << "Texture missing image or sampler index\n";
		newTexture.imageId = texture.imageIndex.value();
		newTexture.samplerId = texture.samplerIndex.value();
		textures.emplace_back(std::make_shared<TextureResource>(std::move(newTexture)));
	}
}

void GltfLoader::loadImages(VulkanRenderer& renderer, const std::filesystem::path& path)
{
	images.resize(asset.images.size());
	createDefaultImages(renderer);
#pragma omp parallel for	
	for (int imgId = 0; imgId < asset.images.size();imgId++) {

		auto& image = asset.images[imgId];
		ImageResource tempImage;
		tempImage.name = image.name;

		std::string imagePath;

		//Load Based on variant type
		if (auto filename = std::get_if<fastgltf::sources::URI>(&image.data)) {
			//type URI just a file path
			imagePath = path.parent_path().string() + "/" + filename->uri.c_str();

			if (!loadImageData(renderer, imagePath, tempImage)) {
				continue;
			}

		}
		else if (fastgltf::sources::Array* imageArray = std::get_if<fastgltf::sources::Array>(&image.data)) {
			//type array an arry of bytes of data
			stbi_uc* bytes = reinterpret_cast<stbi_uc*>(imageArray->bytes.data());

			if (!loadImageData(renderer, bytes, imageArray->bytes.size(), tempImage)) {
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

			if (!loadImageData(renderer, data, size, tempImage)) {
				continue;
			}

		}
		images[imgId] = std::make_shared<ImageResource>(std::move(tempImage));

	
	}
	
}

void GltfLoader::loadMeshes(VulkanRenderer& renderer)
{

	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	drawCalls.resize(asset.meshes.size());
	meshes.reserve(asset.meshes.size());

	for (const auto& mesh : asset.meshes) {

		MeshAsset newMesh;

		newMesh.name = mesh.name;

		indices.clear();
		vertices.clear();
		
		//allocation for drawCall struct
		

		for (const auto& primitive : mesh.primitives) {
			GeoSurface newSurface;
			if(primitive.materialIndex.has_value())
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
			newMesh.surfaces.push_back(newSurface);


		}
		renderer.createMeshResources(vertices, indices, newMesh.meshBuffers);
		meshes.emplace_back(std::make_shared<MeshAsset>(std::move(newMesh)));

	}
}

void GltfLoader::loadGltf(const char* fname, VulkanRenderer& renderer)
{
	std::filesystem::path path = fname;

	constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
		| fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages;

	constexpr fastgltf::Extensions gltfExtensions = fastgltf::Extensions::KHR_lights_punctual| fastgltf::Extensions::KHR_texture_basisu| fastgltf::Extensions::KHR_materials_ior | fastgltf::Extensions::KHR_materials_specular| fastgltf::Extensions::KHR_materials_transmission| fastgltf::Extensions::KHR_materials_emissive_strength;

	fastgltf::Parser parser(gltfExtensions);
	auto data = fastgltf::GltfDataBuffer::FromPath(path);
	if (!data) {
		std::filesystem::path cwd = std::filesystem::current_path();
		std::cout << "Current working directory: " << cwd << std::endl;
		std::cerr << "Failed to load glTF file: " << static_cast<int>(data.error()) << std::endl;
		return;
	}
	auto assetOptional = parser.loadGltf(data.get(), path.parent_path(), gltfOptions );
	if (!assetOptional) {
		std::cerr << "Failed to parse glTF file: " << static_cast<int>(assetOptional.error()) << std::endl;
		return;
	}
	asset = std::move(assetOptional.get());




	loadImages(renderer, path);
	loadSamplers(renderer);
	loadTextures();
	loadMeshes(renderer);
	loadMaterials();
	loadNodes();
	loadLights();
	//findLightParamaters();

	createDescriptorSets(renderer);



}



//Draw functions

void GltfLoader::recordScene(const VulkanRenderer& renderer)  {
	//glm::mat4 indentMatrix(1.0);

	for (auto& drawCall : drawCalls) 
		drawCall.drawCalls.clear();

	lightData.clear();

	glm::mat4 correction = glm::rotate(
		glm::mat4(1.0f),
		glm::radians(90.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	for(auto rootId : rootNodesIds) {
		recordNode(renderer, rootId, correction);
	}

	if (!lightsCreated) {
		VulkanBufferCreateInfo info;
		info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		info.size = sizeof(LightGPUData) * lightData.size();
		info.elementCount = static_cast<uint32_t>(lightData.size());
		info.vmaFlags = VMA_MEMORY_USAGE_GPU_ONLY;


		lightBuffer.createBuffer( renderer.getAllocator(), info);
		lightsCreated = true;
		renderer.bufferStagedUpload(lightBuffer, lightData.data(), static_cast<uint32_t>(sizeof(LightGPUData) * lightData.size()), 1);
		lightDescriptorSet.updateLightDescriptor(lightBuffer, lightData.size());
	}
	
	
}

void  GltfLoader::recordNode(const VulkanRenderer& renderer,const int nodeId, glm::mat4& transforMat)  {
	
	glm::mat4 currentTransform = transforMat * nodes[nodeId]->transform;

	//draw mesh if exists
	if (nodes[nodeId]->meshIndex.has_value()) {
			recordMesh(renderer,nodes[nodeId]->meshIndex.value(), currentTransform);
	}

	//draw mesh if exists
	if (nodes[nodeId]->lightIndex.has_value()) {
		LightGPUData lightDataEntry;
		lightDataEntry.type = lights[nodes[nodeId]->lightIndex.value()]->type;
		lightDataEntry.color = lights[nodes[nodeId]->lightIndex.value()]->color;
		lightDataEntry.intensity = lights[nodes[nodeId]->lightIndex.value()]->intensity;
		lightDataEntry.range = lights[nodes[nodeId]->lightIndex.value()]->range;
		lightDataEntry.spotInnerCos = lights[nodes[nodeId]->lightIndex.value()]->spotInnerCos;
		lightDataEntry.spotOuterCos = lights[nodes[nodeId]->lightIndex.value()]->spotOuterCos;
		lightDataEntry.position = currentTransform * glm::vec4(0, 0, 0, 0);
		lightDataEntry.direction =  currentTransform * glm::vec4(0, -1, 0, 0);

		lightData.emplace_back(std::move(lightDataEntry));
	}

	//call on children
	for (auto childId : nodes[nodeId]->children) {
		recordNode(renderer, childId, currentTransform);
	}

}

void GltfLoader::recordMesh(const VulkanRenderer& renderer, const int meshId, glm::mat4& transform) {
	auto & mesh = meshes[meshId];
	std::array<VulkanImageView*,1> viewsArray;
	std::array<VulkanSampler*, 1> samplerArray;
	//cmdBuff.bindMesh(mesh.get()->meshBuffers.vertexBuffer, mesh.get()->meshBuffers.indexBuffer);
	drawCalls[meshId].indexBuffer = &(mesh->meshBuffers.indexBuffer);
	drawCalls[meshId].vertexBuffer =  &(mesh->meshBuffers.vertexBuffer);
	int prevSize = drawCalls[meshId].drawCalls.size();
	drawCalls[meshId].drawCalls.reserve(prevSize + mesh.get()->surfaces.size());
	//for (auto& surface : mesh.get()->surfaces) {
	for(int i = mesh.get()->surfaces.size() -1; i>= 0;i--){

		VulkanDescriptorSet& auxDescriptor = descriptorSets[mesh.get()->surfaces[i].materialIndex];
		DrawCallData newDrawCall;
		newDrawCall.descriptorSet = &auxDescriptor;
		newDrawCall.mat = (materials[mesh.get()->surfaces[i].materialIndex]).get();
		newDrawCall.transform = transform;

		drawCalls[meshId].drawCalls.emplace_back(std::move(newDrawCall));
		
	}
	
}


void GltfLoader::drawScene(const VulkanRenderer& renderer, VulkanCommandBuffer& cmdBuff) const {



	for (auto& drawCall : drawCalls) {

		
		cmdBuff.bindMesh(*drawCall.vertexBuffer, *drawCall.indexBuffer);
		for (int i = 0; i < drawCall.drawCalls.size();i++) {
			cmdBuff.recordDrawCall(renderer.getGraphicsPipeline(), drawCall,i,lightDescriptorSet);
		}
	}
}




