//
// Created by UserName on 19.02.2024.
//

#ifndef VULKANRENDERENGINE_PIPELINEBUILDER_HPP
#define VULKANRENDERENGINE_PIPELINEBUILDER_HPP

#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include <vulkan/vulkan.hpp>

struct VertexInput {
    uint32_t location;
    uint32_t coordinatesAmount;
    size_t typeSize;
    vk::Format format;
};

struct PushConstantInfo {
    vk::ShaderStageFlags shaderStages;
    size_t size;
};

struct UniformBufferInfo {
    uint32_t binding;
    size_t size;
    uint32_t descriptorCount;
    vk::ShaderStageFlags shaderStages;
};

struct SamplerInfo {
    uint32_t binding;
    uint32_t descriptorCount;
    vk::ShaderStageFlags shaderStages;
};

struct AccelerationStructureInfo{
    uint32_t binding;
    uint32_t descriptorCount;
    vk::ShaderStageFlags shaderStages;
};

struct StorageImageInfo{
    uint32_t binding;
    uint32_t descriptorCount;
    vk::ShaderStageFlags shaderStages;
};

struct StorageBufferInfo{
    uint32_t binding;
    uint32_t descriptorCount;
    vk::ShaderStageFlags stageFlags;
};

class PipelineBuilder {
    friend class PipelineConfigurer;

private:
    std::vector<VertexInput> vertexInputs;
    std::vector<UniformBufferInfo> uniformBufferInfo;
    std::vector<PushConstantInfo> pushConstantInfos;
    std::vector<SamplerInfo> samplersInfo;
    std::vector<AccelerationStructureInfo> accelerationStructuresInfos;
    std::vector<StorageImageInfo> storageImagesInfos;
    std::vector<StorageBufferInfo> storageBufferInfos;
public:
    void addVertexInput(VertexInput input) {
        vertexInputs.push_back(input);
    }

    void addPushConstantInfo(PushConstantInfo info) {
        pushConstantInfos.push_back(info);
    }

    void addUniformBuffer(UniformBufferInfo info) {
        uniformBufferInfo.push_back(info);
    }

    void addStorageBuffer(StorageBufferInfo info){
        storageBufferInfos.push_back(info);
    }

    void addSamplerInfo(SamplerInfo info) {
        samplersInfo.push_back(info);
    }
    void addAccelerationStructureInfo(AccelerationStructureInfo info){
        accelerationStructuresInfos.push_back(info);
    }
    void addStorageImageInfo(StorageImageInfo storageImageInfo){
        storageImagesInfos.push_back(storageImageInfo);
    }
};


#endif //VULKANRENDERENGINE_PIPELINEBUILDER_HPP
