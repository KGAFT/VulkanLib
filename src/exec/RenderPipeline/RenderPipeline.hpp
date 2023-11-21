//
// Created by kgaft on 11/20/23.
//
#pragma once

#include "VulkanLib/GraphicsPipeline/GraphicsPipeline.hpp"
#include "RenderPipelineBuilder.hpp"

class RenderPipeline : IDestroyableObject {
public:
    RenderPipeline(LogicalDevice &device, RenderPipelineBuilder &builder) : device(device),
                                                                            width(builder.width),
                                                                            height(builder.height),
                                                                            attachmentCount(
                                                                                    builder.imagePerStepAmount) {
        renderPass = new RenderPass(builder.swapChain != nullptr,
                                    builder.imagePerStepAmount,
                                    builder.builder, device);
        graphicsPipeline = new GraphicsPipeline(device, builder.shader,
                                                builder.builder,
                                                builder.imagePerStepAmount,
                                                builder.width,
                                                builder.height,
                                                *renderPass);
    }

private:
    GraphicsPipeline *graphicsPipeline;
    RenderPass *renderPass;
    LogicalDevice &device;
    uint32_t width;
    uint32_t height;
    uint32_t attachmentCount;
public:
    void beginRender(vk::CommandBuffer &commandBuffer, FrameBuffer &frameBuffer) {
        renderPass->begin(commandBuffer, frameBuffer, width, height, attachmentCount);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->getGraphicsPipeline());
    }

    void endRender(vk::CommandBuffer &cmd) {
        renderPass->end(cmd);
    }

    void setClearColorValues(float r, float g, float b, float a) {
        vk::ClearColorValue clearColorValue{r, g, b, a};
        renderPass->setClearColorValues(clearColorValue);
    }

public:
    RenderPass *getRenderPass() const {
        return renderPass;
    }

    void recreateForResize(uint32_t width, uint32_t height) {
        graphicsPipeline->recreate(*renderPass, width, height);
    }

    void recreate(RenderPipelineBuilder &builder) {
        destroy();
        destroyed = false;
        renderPass = new RenderPass(builder.swapChain != nullptr,
                                    builder.imagePerStepAmount,
                                    builder.builder, device);
        graphicsPipeline = new GraphicsPipeline(device, builder.shader,
                                                builder.builder,
                                                builder.imagePerStepAmount,
                                                builder.width,
                                                builder.height,
                                                *renderPass);
        width = builder.width;
        height = builder.height;
        attachmentCount =
                builder.imagePerStepAmount;
    }

public:
    void destroy() override {
        destroyed = true;
        delete graphicsPipeline;
        delete renderPass;
    }
};


