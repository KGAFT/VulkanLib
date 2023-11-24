//
// Created by kgaft on 11/14/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "VulkanLib/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"
#include "FrameBuffer.hpp"

class RenderPass : IDestroyableObject{
public:
    RenderPass(bool isForSwapChain, unsigned int attachmentPerStepAmount,
               GraphicsPipelineBuilder *builder, LogicalDevice &device) : device(device) {
        createRenderPass(builder->colorAttachments, isForSwapChain, attachmentPerStepAmount,
                         builder->depthAttachments[0]);
    }

private:
    std::vector<vk::Framebuffer> frameBuffers;
    LogicalDevice &device;
    vk::RenderPass renderPass;
    vk::ClearColorValue clearColorValues{1.0f,0.0f,0.0f,1.0f};
private:
    void createRenderPass(std::vector<std::shared_ptr<ImageView>> &colorAttachments, bool isSwapChainImages,
                          unsigned int attachmentPerStepAmount, std::shared_ptr<ImageView> depthAttachment) {
        std::vector<vk::AttachmentDescription> attachments;
        std::vector<vk::AttachmentReference> references;
        prepareAttachmentDescriptions(colorAttachments, isSwapChainImages, attachmentPerStepAmount, depthAttachment,
                                      attachments);
        prepareAttachmentReferences(colorAttachments, isSwapChainImages, attachmentPerStepAmount, depthAttachment,
                                    references);

        vk::SubpassDescription subpass{};
        subpass.flags = vk::SubpassDescriptionFlags();
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = references.size() - 1;
        subpass.pColorAttachments = references.data();
        subpass.pDepthStencilAttachment = &references[references.size() - 1];
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;

        vk::SubpassDependency dependency{};
        dependency.dstSubpass = 0;
        dependency.dstAccessMask =
                vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependency.dstStageMask =
                vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.srcStageMask =
                vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.flags = vk::RenderPassCreateFlags();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        renderPass = device.getDevice().createRenderPass(renderPassInfo);
    }
public:
    vk::RenderPass &getRenderPass()  {
        return renderPass;
    }

    void begin(vk::CommandBuffer cmd, FrameBuffer& frameBuffer, uint32_t width, uint32_t height, uint32_t attachmentCount){
        std::vector<vk::ClearValue> clearValues;
        clearValues.resize(prepareClearValues(nullptr, attachmentCount));
        prepareClearValues(clearValues.data(), attachmentCount);
        vk::RenderPassBeginInfo beginInfo{};
        beginInfo.renderPass = renderPass;
        beginInfo.framebuffer = frameBuffer.getFrameBuffer();
        beginInfo.renderArea.extent = vk::Extent2D{width, height};
        beginInfo.renderArea.offset = vk::Offset2D{0,0};
        beginInfo.pClearValues = clearValues.data();
        beginInfo.clearValueCount = clearValues.size();
        cmd.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

    }

    void end(vk::CommandBuffer cmd){
        cmd.endRenderPass();
    }

    unsigned int prepareClearValues(vk::ClearValue *result, uint32_t attachmentCount) const
    {
        if (result == nullptr)
        {
            return attachmentCount + 1;
        }
        for (int i = 0; i < attachmentCount; ++i)
        {

            result[i].color = clearColorValues;
        }
        result[attachmentCount].depthStencil = vk::ClearDepthStencilValue{1.0f, (uint32_t)0};
        return attachmentCount+1;
    }

    void setClearColorValues(const vk::ClearColorValue &clearColorValues) {
        RenderPass::clearColorValues = clearColorValues;
    }

private:
    static void prepareAttachmentDescriptions(std::vector<std::shared_ptr<ImageView>> &colorAttachments, bool isSwapChainImages,
                                              unsigned int attachmentPerStepAmount, std::shared_ptr<ImageView> depthAttachment,
                                              std::vector<vk::AttachmentDescription> &output) {
        output.resize(attachmentPerStepAmount + 1);
        for (unsigned int i = 0; i < attachmentPerStepAmount; i++) {
            output[i].flags = vk::AttachmentDescriptionFlags();
            output[i].initialLayout = isSwapChainImages ? vk::ImageLayout::eUndefined
                                                        : colorAttachments[i]->getParentInfo().initialLayout;
            output[i].finalLayout = isSwapChainImages ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eGeneral;
            output[i].loadOp = vk::AttachmentLoadOp::eClear;
            output[i].storeOp = vk::AttachmentStoreOp::eStore;
            output[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            output[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            output[i].format = isSwapChainImages ? SwapChain::getFormat().format
                                                 : colorAttachments[i]->getParentInfo().format;
        }
        output[attachmentPerStepAmount].flags = vk::AttachmentDescriptionFlags();
        output[attachmentPerStepAmount].finalLayout = vk::ImageLayout::eGeneral;
        output[attachmentPerStepAmount].initialLayout = depthAttachment->getParentInfo().initialLayout;
        output[attachmentPerStepAmount].loadOp = vk::AttachmentLoadOp::eClear;
        output[attachmentPerStepAmount].storeOp = vk::AttachmentStoreOp::eStore;
        output[attachmentPerStepAmount].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        output[attachmentPerStepAmount].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        output[attachmentPerStepAmount].format = depthAttachment->getParentInfo().format;
    }

    static void prepareAttachmentReferences(std::vector<std::shared_ptr<ImageView>> &colorAttachments, bool isSwapChainImages,
                                            unsigned int attachmentPerStepAmount, std::shared_ptr<ImageView> depthAttachment,
                                            std::vector<vk::AttachmentReference> &output) {
        output.resize(attachmentPerStepAmount + 1);
        for (int i = 0; i < attachmentPerStepAmount; ++i) {
            output[i].attachment = i;
            output[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
        }
        output[attachmentPerStepAmount].layout = vk::ImageLayout::eGeneral;
        output[attachmentPerStepAmount].attachment = attachmentPerStepAmount;
    }
public:
    void destroy() override {
        destroyed = true;
        device.getDevice().destroyRenderPass(renderPass);
    }
};



