//
// Created by kgaft on 11/11/23.
//
#pragma once

#include "VulkanLib/Pipelines/GraphicsPipeline/Configuration/GraphicsPipelineConfig.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Pipelines/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/Pipelines/PipelineConfiguration/PipelineConfigurer.hpp"
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
                     unsigned int width, unsigned int height) :
            attachmentsFormats(builder->colorAttachmentInfo),
            depthFormat(builder->depthAttachmentInfo), configurer(device, &builder->pipelineBuilder), device(device),
            shader(shader), attachmentPerStepAmount(attachmentPerStepAmount) {
        create(attachmentPerStepAmount, width, height, shader, attachmentsFormats, depthFormat);
    }

private:
    std::vector<vk::Format> attachmentsFormats;
    vk::Format depthFormat;
    PipelineConfigurer configurer;
    vk::Pipeline graphicsPipeline;
    LogicalDevice &device;
    Shader *shader;
    unsigned int attachmentPerStepAmount;
public:
    vk::Pipeline getGraphicsPipeline() {
        return graphicsPipeline;
    }

    void recreate(unsigned int width, unsigned int height) {
        device.getDevice().destroyPipeline(graphicsPipeline);
        create(attachmentPerStepAmount, width, height, shader, attachmentsFormats, depthFormat);
    }

    vk::DescriptorSetLayout getDescriptorLayout() {
        return configurer.descriptorSetLayout;
    }

    vk::PipelineLayout getPipelineLayout() {
        return configurer.pipelineLayout;
    }

private:
    void create(unsigned int attachmentPerStepAmount, unsigned int width, unsigned int height, Shader *shader,
                std::vector<vk::Format> colorFormats, vk::Format depthFormat) {
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
        createStrip->pipelineInfo.subpass = config->subpass;

        createStrip->pipelineInfo.basePipelineIndex = -1;
        createStrip->pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        vk::PipelineRenderingCreateInfo renderingCreateInfo{};
        renderingCreateInfo.colorAttachmentCount = (uint32_t) colorFormats.size();
        renderingCreateInfo.pColorAttachmentFormats = colorFormats.data();
        renderingCreateInfo.depthAttachmentFormat = depthFormat;

        createStrip->pipelineInfo.pNext = &renderingCreateInfo;

        graphicsPipeline = device.getDevice().createGraphicsPipeline(nullptr, createStrip->pipelineInfo).value;
        configInstancer.releaseObjectInstance(config);
        createInstance.releaseObjectInstance(createStrip);
    }


public:
    void destroy() override {
        destroyed = true;
        device.getDevice().destroyPipeline(graphicsPipeline);
        configurer.destroy();
    }
};



