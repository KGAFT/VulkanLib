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

class GraphicsPipelineBuilder {
    friend class GraphicsPipelineConfigurer;

    friend class RenderPass;

private:
    static inline SeriesObject<GraphicsPipelineBuilder> builders = SeriesObject<GraphicsPipelineBuilder>();
public:
    static GraphicsPipelineBuilder *getInstance() {
        return builders.getObjectInstance();
    }

public:
    GraphicsPipelineBuilder() {

    }

private:
    std::vector<std::shared_ptr<ImageView>> colorAttachments;
    std::vector<std::shared_ptr<ImageView>> depthAttachments;
    std::vector<VertexInput> vertexInputs;
    std::vector<UniformBufferInfo> uniformBufferInfo;
    std::vector<PushConstantInfo> pushConstantInfos;
    std::vector<SamplerInfo> samplersInfo;
public:
    void addColorAttachment(std::shared_ptr<ImageView> attachment) {
        colorAttachments.push_back(attachment);
    }

    void addColorAttachments(std::shared_ptr<ImageView> *pAttachments, unsigned int attachmentCount) {
        colorAttachments.resize(attachmentCount);
        for (unsigned int i = 0; i < attachmentCount; ++i) {
            colorAttachments[i] = pAttachments[i];
        }
    }

    void addDepthAttachments(std::shared_ptr<ImageView> *pDepthAttachments, unsigned int attachmentCount) {
        depthAttachments.resize(attachmentCount);
        for (unsigned int i = 0; i < attachmentCount; ++i) {
            depthAttachments[i] = pDepthAttachments[i];
        }
    }

    void addDepthAttachment(std::shared_ptr<ImageView>attachment) {
        depthAttachments.push_back(attachment);
    }

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

    /**
     * @throws runtime_error if is not properly populated
     */
    void checkIfBuilderComplete() {
        if (colorAttachments.size() < 3) {
            throw std::runtime_error("Error: you need to add at least three color attachment");
        }
        if (vertexInputs.empty()) {
            throw std::runtime_error("Error: you need to add at least one vertex input");
        }
    }

public:
    void release() {
        builders.releaseObjectInstance(this);
    }
};



