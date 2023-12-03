//
// Created by kgaft on 11/14/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "VulkanLib/GraphicsPipeline/Configuration/GraphicsPipelineBuilder.hpp"
#include "VulkanLib/Device/SwapChain/SwapChain.hpp"
#include "FrameBuffer.hpp"
#include "RenderPassBuilder.hpp"

class RenderPass : IDestroyableObject{
public:
    RenderPass(RenderPassBuilder& builder,  std::shared_ptr<LogicalDevice> device) : device(device) {
        createRenderPass(builder);
    }

private:
    std::vector<vk::Framebuffer> frameBuffers;
    std::shared_ptr<LogicalDevice> device;
    vk::RenderPass renderPass;
    vk::ClearColorValue clearColorValues{1.0f,0.0f,0.0f,1.0f};
private:
    void createRenderPass(RenderPassBuilder& builder) {
        std::vector<vk::AttachmentDescription> attachments;
        std::vector<vk::AttachmentReference> references;
        uint32_t i = 0;
        for (auto &item: builder.subPasses){
            for (auto &citem: item.inputAttachments){
                attachments.push_back(citem.description);
                citem.i = i;
                references.push_back({i, citem.layout});
                i++;
            }
            for ( auto &citem: item.outputAttachments){
                attachments.push_back(citem.description);
                citem.i = i;
                references.push_back({i, citem.layout});
                i++;
            }
            attachments.push_back(item.outputDepthAttachment.description);
            item.outputDepthAttachment.i = i;
            references.push_back({i, item.outputDepthAttachment.layout});
            i++;
        }
        std::vector<vk::SubpassDescription> descriptions;
        descriptions.resize(builder.subPasses.size());
        for (uint32_t i = 0; i < builder.subPasses.size(); ++i){
            descriptions[i].flags = vk::SubpassDescriptionFlags();
            descriptions[i].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            descriptions[i].inputAttachmentCount = builder.subPasses[i].inputAttachments.size();
            descriptions[i].pInputAttachments = &references[builder.subPasses[i].inputAttachments[0].i];

            descriptions[i].pColorAttachments = &references[builder.subPasses[i].outputAttachments[0].i];
            descriptions[i].colorAttachmentCount = builder.subPasses[i].outputAttachments.size();

            descriptions[i].pDepthStencilAttachment = &references[builder.subPasses[i].outputDepthAttachment.i];
        }
        std::vector<vk::SubpassDependency> dependencies;
        dependencies.resize(descriptions.size()+1);
        for (uint32_t i = 0; i < dependencies.size(); ++i){
            if(i == 0){
                dependencies[i].dstSubpass = 0;
                dependencies[i].dstAccessMask =
                        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                dependencies[i].dstStageMask =
                        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
                dependencies[i].srcSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[i].srcAccessMask = vk::AccessFlagBits::eNone;
                dependencies[i].srcStageMask =
                        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            } else if (i == dependencies.size()-1){
                dependencies[i].srcSubpass = i-1;
                dependencies[i].dstSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[i].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                dependencies[i].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
                dependencies[i].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
                dependencies[i].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
                dependencies[i].dependencyFlags = vk::DependencyFlagBits::eByRegion;
            } else {
                dependencies[i].srcSubpass = i-1;
                dependencies[i].dstSubpass = i;
                dependencies[i].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput ;
                dependencies[i].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
                dependencies[i].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                dependencies[i].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
                dependencies[i].dependencyFlags = vk::DependencyFlagBits::eByRegion;
            }
        }



        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.flags = vk::RenderPassCreateFlags();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.subpassCount = descriptions.size();
        renderPassInfo.pSubpasses = descriptions.data();
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

        renderPass = device->getDevice().createRenderPass(renderPassInfo);
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


public:
    void destroy() override {
        destroyed = true;
        device->getDevice().destroyRenderPass(renderPass);
    }
};



