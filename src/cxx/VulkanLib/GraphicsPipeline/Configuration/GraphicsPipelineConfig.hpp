//
// Created by kgaft on 11/11/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/MemoryUtils.hpp"

class GraphicsPipelineConfig{
private:
public:
    static void createConfig(GraphicsPipelineConfig* configInfo, unsigned int attachmentCount, bool alphaBlending, unsigned int width, unsigned int height){
        configInfo->inputAssemblyInfo.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        configInfo->inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        configInfo->inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        configInfo->viewport.x = 0.0f;
        configInfo->viewport.y = 0.0f;
        configInfo->viewport.width = static_cast<float>(width);
        configInfo->viewport.height = static_cast<float>(height);
        configInfo->viewport.minDepth = 0.0f;
        configInfo->viewport.maxDepth = 1.0f;
        configInfo->scissor.offset = vk::Offset2D(0,0);
        configInfo->scissor.extent = vk::Extent2D(width, height);
        configInfo->rasterizationInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        configInfo->rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo->rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo->rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
        configInfo->rasterizationInfo.lineWidth = 1.0f;
        configInfo->rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
        configInfo->rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
        configInfo->rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo->rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
        configInfo->rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        configInfo->rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional
        configInfo->multisampleInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        configInfo->multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo->multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        configInfo->multisampleInfo.minSampleShading = 1.0f;          // Optional
        configInfo->multisampleInfo.pSampleMask = nullptr;            // Optional
        configInfo->multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        configInfo->multisampleInfo.alphaToOneEnable = VK_FALSE;
        configInfo->colorBlendAttachments.clear();
        configInfo->colorBlendAttachments.resize(attachmentCount);
        if(alphaBlending){

            for (uint32_t i = 0; i < attachmentCount; ++i){
                configInfo->colorBlendAttachments[i].blendEnable = VK_TRUE;
                configInfo->colorBlendAttachments[i].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
                configInfo->colorBlendAttachments[i].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
                configInfo->colorBlendAttachments[i].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
                configInfo->colorBlendAttachments[i].colorBlendOp = vk::BlendOp::eAdd;
                configInfo->colorBlendAttachments[i].srcAlphaBlendFactor = vk::BlendFactor::eOne;
                configInfo->colorBlendAttachments[i].dstAlphaBlendFactor = vk::BlendFactor::eZero;
                configInfo->colorBlendAttachments[i].alphaBlendOp = vk::BlendOp::eAdd;
            }
        }
        else{
            for (uint32_t i = 0; i < attachmentCount; ++i){
                configInfo->colorBlendAttachments[i].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                                    vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
                configInfo->colorBlendAttachments[i].blendEnable = VK_FALSE;
                configInfo->colorBlendAttachments[i].srcColorBlendFactor = vk::BlendFactor::eOne;   // Optional
                configInfo->colorBlendAttachments[i].dstColorBlendFactor = vk::BlendFactor::eZero;  // Optional
                configInfo->colorBlendAttachments[i].colorBlendOp = vk::BlendOp::eAdd;              // Optional
                configInfo->colorBlendAttachments[i].srcAlphaBlendFactor = vk::BlendFactor::eOne;
                configInfo->colorBlendAttachments[i].dstAlphaBlendFactor = vk::BlendFactor::eZero;
                configInfo->colorBlendAttachments[i].alphaBlendOp = vk::BlendOp::eAdd;
            }
        }


        configInfo->colorBlendInfo.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        configInfo->colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo->colorBlendInfo.logicOp = vk::LogicOp::eCopy;
        configInfo->colorBlendInfo.attachmentCount = attachmentCount;
        configInfo->colorBlendInfo.pAttachments = configInfo->colorBlendAttachments.data();
        configInfo->colorBlendInfo.blendConstants[0] = 0.0f;
        configInfo->colorBlendInfo.blendConstants[1] = 0.0f;
        configInfo->colorBlendInfo.blendConstants[2] = 0.0f;
        configInfo->colorBlendInfo.blendConstants[3] = 0.0f;
        configInfo->depthStencilInfo.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
        configInfo->depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo->depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo->depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
        configInfo->depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo->depthStencilInfo.minDepthBounds = 0.0f;
        configInfo->depthStencilInfo.maxDepthBounds = 1.0f;
        configInfo->depthStencilInfo.stencilTestEnable = VK_FALSE;

    }
public:
    vk::Viewport viewport;
    vk::Rect2D scissor;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;

    uint32_t subpass = 0;
};