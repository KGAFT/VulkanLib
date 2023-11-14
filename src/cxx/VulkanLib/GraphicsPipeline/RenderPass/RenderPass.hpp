//
// Created by kgaft on 11/14/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "VulkanLib/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"

class RenderPass : IDestroyableObject{
public:
    RenderPass(bool isForSwapChain, unsigned int attachmentPerStepAmount,
               GraphicsPipelineBuilder &builder, LogicalDevice &device) : device(device) {
        createRenderPass(builder.colorAttachments, isForSwapChain, attachmentPerStepAmount,
                         builder.depthAttachments[0]);
    }

private:
    std::vector<vk::Framebuffer> frameBuffers;
    LogicalDevice &device;
    vk::RenderPass renderPass;
private:
    void createRenderPass(std::vector<ImageView *> &colorAttachments, bool isSwapChainImages,
                          unsigned int attachmentPerStepAmount, ImageView *depthAttachment) {
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
    const vk::RenderPass &getRenderPass() const {
        return renderPass;
    }

private:
    static void prepareAttachmentDescriptions(std::vector<ImageView *> &colorAttachments, bool isSwapChainImages,
                                              unsigned int attachmentPerStepAmount, ImageView *depthAttachment,
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

    static void prepareAttachmentReferences(std::vector<ImageView *> &colorAttachments, bool isSwapChainImages,
                                            unsigned int attachmentPerStepAmount, ImageView *depthAttachment,
                                            std::vector<vk::AttachmentReference> &output) {
        output.resize(attachmentPerStepAmount + 1);
        for (int i = 0; i < attachmentPerStepAmount; ++i) {
            output[i].attachment = i;
            output[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
        }
        output[attachmentPerStepAmount].layout = vk::ImageLayout::eGeneral;
        output[attachmentPerStepAmount].attachment = attachmentPerStepAmount;
    }

    void destroy() override {
        destroyed = true;
        device.getDevice().destroyRenderPass(renderPass);
    }
};



