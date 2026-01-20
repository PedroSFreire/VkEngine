#pragma once

#include "fastgltf/tools.hpp"
#include "fastgltf/core.hpp"
#include "fastgltf/util.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "fastgltf/base64.hpp"


#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <span>
#include <array>
#include <string_view>
#include "glm/glm.hpp"

#include "../Core/defines.h"

#include "mikktspace.h"




class VulkanCommandBuffer;

class  VulkanRenderer;
class Scene;

class GltfLoader
{
private:

    fastgltf::Asset asset;

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

	//load file in fname into Scene
    void loadGltf(SceneData& Scene, const char* fname);

private:
	//mikktspace context
    SMikkTSpaceContext ctx = {};

	//load functions
    void loadMeshes(SceneData& Scene);
    void loadImages(SceneData& Scene, const std::filesystem::path& path);
    void loadSamplers(SceneData& Scene);
    void loadTextures(SceneData& Scene);
    void loadMaterials(SceneData& Scene);
    void loadNodes(SceneData& Scene);
    void loadLights(SceneData& Scene);

	//Load helpers
    LightType getLightType(fastgltf::LightType gltfType);

    //load Helpers
    void loadNodesData(SceneData& Scene);
    void loadNodesRelatrions(SceneData& Scene);
    bool loadImageData(SceneData& scene, stbi_uc* bytes, uint32_t size, ImageAsset& tempTexture, int id);
    bool loadImageData(SceneData& scene, const std::string& TEXTURE_PATH, ImageAsset& tempImage, int id);

    Filter getFilter(fastgltf::Filter gltfFilter);
    MipmapMode getFilterMode(fastgltf::Filter gltfFilter);
    AddressMode getWrap(fastgltf::Wrap wrap);
   


};