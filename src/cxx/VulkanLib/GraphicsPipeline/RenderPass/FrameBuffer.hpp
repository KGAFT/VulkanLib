//
// Created by kgaft on 11/14/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/Device/Image/ImageView.hpp"

class FrameBuffer : IDestroyableObject {
public:
    FrameBuffer(std::shared_ptr<LogicalDevice> device, vk::RenderPass& renderPass, std::shared_ptr<ImageView> *pImages, uint32_t imageCount, uint32_t width,
                uint32_t height) : device(device), renderPass(renderPass) {
        create(pImages, imageCount, width, height);
    }

private:
    vk::Framebuffer frameBuffer;
    vk::RenderPass& renderPass;
    std::shared_ptr<LogicalDevice> device;
public:
    void recreate(std::shared_ptr<ImageView> *pImages, uint32_t imageCount,
                  uint32_t width, uint32_t height){
        destroy();
        destroyed = false;
        create(pImages, imageCount, width, height);
    }

    const vk::Framebuffer &getFrameBuffer() const {
        return frameBuffer;
    }

private:
    void create(std::shared_ptr<ImageView> *pImages, uint32_t imageCount,
                uint32_t width, uint32_t height) {
        std::vector<vk::ImageView> views;
        views.resize(imageCount);
        for (int i = 0; i < imageCount; ++i) {
            views[i] = pImages[i]->getBase();
        }
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.flags = vk::FramebufferCreateFlags();
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = views.size();
        framebufferInfo.pAttachments = views.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        frameBuffer = device->getDevice().createFramebuffer(framebufferInfo);
    }

    void destroy() override {
        destroyed = true;
        device->getDevice().destroyFramebuffer(frameBuffer);

    }
};


