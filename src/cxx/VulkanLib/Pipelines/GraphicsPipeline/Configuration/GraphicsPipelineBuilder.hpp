//
// Created by kgaft on 11/11/23.
//
#pragma once

#include <vector>
#include "VulkanLib/Device/Image/ImageView.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include "VulkanLib/Pipelines/PipelineConfiguration/PipelineBuilder.hpp"



class GraphicsPipelineBuilder : public IDestroyableObject {
    friend class PipelineConfigurer;

    friend class GraphicsRenderPipeline;

    friend class GraphicsPipeline;

private:
    static inline SeriesObject<GraphicsPipelineBuilder> builders = SeriesObject<GraphicsPipelineBuilder>();

public:

    GraphicsPipelineBuilder() {

    }

private:
    PipelineBuilder pipelineBuilder;
    std::vector<vk::Format> colorAttachmentInfo;
    vk::Format depthAttachmentInfo;
public:


    void addVertexInput(VertexInput input) {
        pipelineBuilder.addVertexInput(input);
    }

    void addStorageBufferInfo(StorageBufferInfo info){
        pipelineBuilder.addStorageBuffer(info);
    }

    void addPushConstantInfo(PushConstantInfo info) {
        pipelineBuilder.addPushConstantInfo(info);
    }

    void addUniformBuffer(UniformBufferInfo info) {
        pipelineBuilder.addUniformBuffer(info);
    }

    void addSamplerInfo(SamplerInfo info) {
        pipelineBuilder.addSamplerInfo(info);
    }

    void addColorAttachmentInfo(vk::Format colorAttachmentFormat) {
        colorAttachmentInfo.push_back(colorAttachmentFormat);
    }

    void setDepthAttachmentInfo(vk::Format depthAttachmentInfo) {
        this->depthAttachmentInfo = depthAttachmentInfo;
    }
    void addStorageImageInfo(StorageImageInfo storageImageInfo){
        pipelineBuilder.addStorageImageInfo(storageImageInfo);
    }

public:
    void destroy() override {
        destroyed = true;
        builders.releaseObjectInstance(this);
    }

    virtual ~GraphicsPipelineBuilder() {

    }


};



