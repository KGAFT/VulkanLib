//
// Created by kgaft on 11/11/23.
//
#pragma once

#include <vector>
#include "VulkanLib/Device/Image/ImageView.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"

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

class GraphicsPipelineBuilder : public IDestroyableObject {
    friend class GraphicsPipelineConfigurer;

    friend class GraphicsRenderPipeline;

    friend class GraphicsPipeline;

private:
    static inline SeriesObject<GraphicsPipelineBuilder> builders = SeriesObject<GraphicsPipelineBuilder>();
public:
    static GraphicsPipelineBuilder *getInstance() {
        return builders.getObjectInstance();
    }
    static void releaseBuilderInstance(GraphicsPipelineBuilder* builder){
        builders.releaseObjectInstance(builder);
    }
public:
    GraphicsPipelineBuilder() {

    }

private:

    std::vector<VertexInput> vertexInputs;
    std::vector<UniformBufferInfo> uniformBufferInfo;
    std::vector<PushConstantInfo> pushConstantInfos;
    std::vector<SamplerInfo> samplersInfo;
    std::vector<vk::Format> colorAttachmentInfo;
    vk::Format depthAttachmentInfo;
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

    void addColorAttachmentInfo(vk::Format colorAttachmentFormat) {
        colorAttachmentInfo.push_back(colorAttachmentFormat);
    }


    void setDepthAttachmentInfo(vk::Format depthAttachmentInfo) {
        this->depthAttachmentInfo = depthAttachmentInfo;
    }

    /**
     * @throws runtime_error if is not properly populated
     */
    void checkIfBuilderComplete() {

        if (vertexInputs.empty()) {
            throw std::runtime_error("Error: you need to add at least one vertex input");
        }
    }

public:
    void destroy() override {
        destroyed = true;
        builders.releaseObjectInstance(this);
    }

    virtual ~GraphicsPipelineBuilder() {

    }


};



