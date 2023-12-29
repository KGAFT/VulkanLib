//
// Created by kgaft on 11/14/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>
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
    std::shared_ptr<LogicalDevice> device;
    vk::RenderPass renderPass;
private:
    void createRenderPass(RenderPassBuilder& builder) {
        std::vector<vk::AttachmentDescription> descriptions;
        std::vector<vk::AttachmentReference> references;
        std::vector<vk::SubpassDescription> subPassDescriptions;
        subPassDescriptions.resize(builder.subPasses.size());
        uint32_t c = 0;
        for (auto &item: builder.subPasses){
            copyAttachmentsInfo(item, descriptions);
            uint32_t counter = descriptions.size()-1-item.inputAttachments.size()-item.outputAttachments.size()-1;
            for (auto &citem: item.inputAttachments){
                references.push_back({counter, vk::ImageLayout::eShaderReadOnlyOptimal});
                counter++;
            }
            for (auto &citem: item.outputAttachments){
                references.push_back({counter, citem.finalLayout});
                counter++;
            }
            references.push_back({counter, item.depthOutDescription.finalLayout});
            MemoryUtils::memClear(&subPassDescriptions[c], sizeof(vk::SubpassDescription));
            subPassDescriptions[c].pInputAttachments = &references[descriptions.size()-1-item.inputAttachments.size()-item.outputAttachments.size()-1];
            subPassDescriptions[c].pColorAttachments = &references[descriptions.size()-1-item.outputAttachments.size()-1];
            subPassDescriptions[c].pDepthStencilAttachment = &references[descriptions.size()-1];
            subPassDescriptions[c].inputAttachmentCount = item.inputAttachments.size();
            subPassDescriptions[c].colorAttachmentCount = item.outputAttachments.size();
            c++;
        }


    }

public:

private:
    void copyAttachmentsInfo(SubPass& subPass, std::vector<vk::AttachmentDescription>& output){
        for (auto &citem: subPass.inputAttachments){
            output.push_back(citem);
        }
        for (auto &citem: subPass.outputAttachments){
            output.push_back(citem);
        }
        output.push_back(subPass.depthOutDescription);
    }


public:
    void destroy() override {
        destroyed = true;
        device->getDevice().destroyRenderPass(renderPass);
    }
};



