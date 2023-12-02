//
// Created by kgaft on 11/11/23.
//
#pragma once

#include "Configuration/GraphicsPipelineConfig.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/GraphicsPipeline/RenderPass/RenderPass.hpp"
#include "VulkanLib/GraphicsPipeline/Configuration/GraphicsPipelineConfigurer.hpp"
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include "Shader.hpp"

struct GraphicsPipelineCreateStrip {
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vk::PipelineViewportStateCreateInfo viewportInfo{};

    vk::GraphicsPipelineCreateInfo pipelineInfo{};

};

class GraphicsPipeline : public IDestroyableObject {
private:
    static inline SeriesObject<GraphicsPipelineConfig> configInstancer = SeriesObject<GraphicsPipelineConfig>();
    static inline SeriesObject<GraphicsPipelineCreateStrip> createInstance = SeriesObject<GraphicsPipelineCreateStrip>();
public:
    GraphicsPipeline(LogicalDevice &device, Shader *shader, GraphicsPipelineBuilder *builder,
                     unsigned int attachmentPerStepAmount,
                     unsigned int width, unsigned int height,
                     RenderPass &renderPass) : device(device), configurer(device, builder), shader(shader),
                                               attachmentPerStepAmount(attachmentPerStepAmount) {
        create(attachmentPerStepAmount, width, height, shader, renderPass);
    }

private:
    GraphicsPipelineConfigurer configurer;
    vk::Pipeline graphicsPipeline;
    LogicalDevice &device;
    Shader *shader;
    unsigned int attachmentPerStepAmount;
public:
    vk::Pipeline getGraphicsPipeline() {
        return graphicsPipeline;
    }

    void recreate(RenderPass &renderPass, unsigned int width, unsigned int height) {
        destroy();
        destroyed = false;
        create(attachmentPerStepAmount, width, height, shader, renderPass);
    }

    vk::DescriptorSetLayout getDescriptorLayout(){
        return configurer.descriptorSetLayout;
    }
    vk::PipelineLayout getPipelineLayout(){
        return configurer.pipelineLayout;
    }

private:
    void create(unsigned int attachmentPerStepAmount, unsigned int width, unsigned int height, Shader *shader,
                RenderPass &renderPass) {
        GraphicsPipelineConfig *config = configInstancer.getObjectInstance();
        GraphicsPipelineCreateStrip *createStrip = createInstance.getObjectInstance();
        GraphicsPipelineConfig::createConfig(config, attachmentPerStepAmount, true, width, height);

        createStrip->vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        createStrip->vertexInputInfo.vertexAttributeDescriptionCount = configurer.inputAttribDescs.size();
        createStrip->vertexInputInfo.vertexBindingDescriptionCount = 1;
        createStrip->vertexInputInfo.pVertexAttributeDescriptions = configurer.inputAttribDescs.data();
        createStrip->vertexInputInfo.pVertexBindingDescriptions = &configurer.inputBindDesc;

        createStrip->viewportInfo.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
        createStrip->viewportInfo.viewportCount = 1;
        createStrip->viewportInfo.pViewports = &config->viewport;
        createStrip->viewportInfo.scissorCount = 1;
        createStrip->viewportInfo.pScissors = &config->scissor;

        createStrip->pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        createStrip->pipelineInfo.stageCount = shader->getCreateInfos().size();
        createStrip->pipelineInfo.pStages = shader->getCreateInfos().data();
        createStrip->pipelineInfo.pVertexInputState = &createStrip->vertexInputInfo;
        createStrip->pipelineInfo.pInputAssemblyState = &config->inputAssemblyInfo;
        createStrip->pipelineInfo.pViewportState = &createStrip->viewportInfo;
        createStrip->pipelineInfo.pRasterizationState = &config->rasterizationInfo;
        createStrip->pipelineInfo.pMultisampleState = &config->multisampleInfo;
        createStrip->pipelineInfo.pColorBlendState = &config->colorBlendInfo;
        createStrip->pipelineInfo.pDepthStencilState = &config->depthStencilInfo;
        createStrip->pipelineInfo.pDynamicState = nullptr;

        createStrip->pipelineInfo.layout = configurer.pipelineLayout;
        createStrip->pipelineInfo.renderPass = renderPass.getRenderPass();
        createStrip->pipelineInfo.subpass = config->subpass;

        createStrip->pipelineInfo.basePipelineIndex = -1;
        createStrip->pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipeline = device.getDevice().createGraphicsPipeline(nullptr, createStrip->pipelineInfo).value;
        configInstancer.releaseObjectInstance(config);
        createInstance.releaseObjectInstance(createStrip);
    }


public:
    void destroy() override {
        destroyed = true;
        device.getDevice().destroyPipeline(graphicsPipeline);
    }
};



