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
#include "VulkanLib/MemoryUtils/SerialObject.hpp"
#include "Shader.hpp"

class GraphicsPipeline : public IDestroyableObject {
private:
    static inline SerialObject<GraphicsPipelineConfig> configInstancer = SerialObject<GraphicsPipelineConfig>();
public:
    GraphicsPipeline(LogicalDevice &device, Shader *shader, GraphicsPipelineBuilder *builder,
                     unsigned int attachmentPerStepAmount,
                     unsigned int width, unsigned int height,
                     RenderPass &renderPass) : device(device), configurer(device, builder), shader(shader), attachmentPerStepAmount(attachmentPerStepAmount) {
        create(attachmentPerStepAmount, width, height, shader, renderPass);
    }

private:
    GraphicsPipelineConfigurer configurer;
    vk::Pipeline graphicsPipeline;
    LogicalDevice &device;
    Shader* shader;
    unsigned int attachmentPerStepAmount;
public:
    vk::Pipeline getGraphicsPipeline() {
        return graphicsPipeline;
    }
    void recreate( RenderPass& renderPass, unsigned int width, unsigned int height){
        destroy();
        destroyed = false;
        create(attachmentPerStepAmount, width, height, shader, renderPass);
    }
private:
    void create(unsigned int attachmentPerStepAmount, unsigned int width, unsigned int height, Shader* shader, RenderPass& renderPass){
        GraphicsPipelineConfig *config = configInstancer.getObjectInstance();
        GraphicsPipelineConfig::createConfig(config, attachmentPerStepAmount, true, width, height);

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexAttributeDescriptionCount = configurer.inputAttribDescs.size();
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexAttributeDescriptions = configurer.inputAttribDescs.data();
        vertexInputInfo.pVertexBindingDescriptions = &configurer.inputBindDesc;

        vk::PipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &config->viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &config->scissor;

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.stageCount = shader->getCreateInfos().size();
        pipelineInfo.pStages = shader->getCreateInfos().data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &config->inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &config->rasterizationInfo;
        pipelineInfo.pMultisampleState = &config->multisampleInfo;
        pipelineInfo.pColorBlendState = &config->colorBlendInfo;
        pipelineInfo.pDepthStencilState = &config->depthStencilInfo;
        pipelineInfo.pDynamicState = nullptr;

        pipelineInfo.layout = configurer.pipelineLayout;
        pipelineInfo.renderPass = renderPass.getRenderPass();
        pipelineInfo.subpass = config->subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
        configInstancer.releaseObjectInstance(config);
    }
public:
    void destroy() override {
        destroyed = true;
        device.getDevice().destroyPipeline(graphicsPipeline);
    }
};



