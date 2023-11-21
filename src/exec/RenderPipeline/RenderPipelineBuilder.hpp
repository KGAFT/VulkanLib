//
// Created by kgaft on 11/19/23.
//
#pragma once

#include "VulkanLib/Device/Image/ImageView.hpp"
#include "VulkanLib/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"
#include "VulkanLib/GraphicsPipeline/Shader.hpp"

class RenderPipelineBuilder {
    friend class RenderPipeline;
public:
    RenderPipelineBuilder(){

    }
private:
    GraphicsPipelineBuilder* builder;
    SwapChain* swapChain = nullptr;
    uint32_t imagePerStepAmount;
    Shader* shader;
    uint32_t width;
    uint32_t height;
public:
    void setSwapChain(SwapChain* swapChain){
        RenderPipelineBuilder::swapChain = swapChain;
    }
    void setImagePerStepAmount(uint32_t imagePerStepAmount){
        RenderPipelineBuilder::imagePerStepAmount = imagePerStepAmount;
    }

    void setGraphicsPipelineBuilder(GraphicsPipelineBuilder* builder){
        RenderPipelineBuilder::builder = builder;
    }

    void setShader(Shader *shader) {
        RenderPipelineBuilder::shader = shader;
    }

    void setWidth(uint32_t width) {
        RenderPipelineBuilder::width = width;
    }

    void setHeight(uint32_t height) {
        RenderPipelineBuilder::height = height;
    }
};


