#pragma once


#include "tiny_obj_loader.h"

#include "defines.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class ModelLoader
{
private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

public:
    ModelLoader() = default;
    ~ModelLoader() = default;
    ModelLoader(const ModelLoader&) = delete;

    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }

    void loadModel(const std::string filePath);

};

