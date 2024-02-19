//
// Created by UserName on 19.02.2024.
//

#ifndef VULKANRENDERENGINE_PIPELINEBUILDER_HPP
#define VULKANRENDERENGINE_PIPELINEBUILDER_HPP

#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include <vulkan/vulkan.hpp>

struct VertexInput {
    unsigned int location;
    unsigned int coordinatesAmount;
    size_t typeSize;
    vk::Format format;
};

struct PushConstantInfo {
    vk::ShaderStageFlags shaderStages;
    size_t size;
};

struct UniformBufferInfo {
    int binding;
    size_t size;
    vk::ShaderStageFlags shaderStages;
};

struct SamplerInfo {
    int binding;
    vk::ShaderStageFlags shaderStages;
};

struct AccelerationStructureInfo{
    int binding;
    vk::ShaderStageFlags shaderStages;
};

struct StorageImageInfo{
    int binding;
    vk::ShaderStageFlags shaderStages;
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
