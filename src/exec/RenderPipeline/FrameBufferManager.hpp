//
// Created by kgaft on 11/23/23.
//
#pragma once

#include <VulkanLib/Device/Image/Image.hpp>
#include <memory>
#include <utility>
#include "VulkanLib/GraphicsPipeline/RenderPass/FrameBuffer.hpp"

class FrameBufferManager : IDestroyableObject {
private:
    static inline vk::ImageCreateInfo defaultColorCreateInfo = {vk::ImageCreateFlags(),
                                                                vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb,
                                                                vk::Extent3D{800, 600, 1},
                                                                1, 1, vk::SampleCountFlagBits::e1,
                                                                vk::ImageTiling::eOptimal,
                                                                vk::ImageUsageFlagBits::eColorAttachment |
                                                                vk::ImageUsageFlagBits::eSampled |
                                                                vk::ImageUsageFlagBits::eTransferSrc,
                                                                vk::SharingMode::eExclusive,
                                                                0, 0,
                                                                vk::ImageLayout::eUndefined, nullptr

    };
    static inline vk::ImageViewCreateInfo defaultColorViewCreateInfo = {vk::ImageViewCreateFlags(),
                                                                        nullptr, vk::ImageViewType::e2D,
                                                                        vk::Format::eR8G8B8A8Srgb, {},
                                                                        vk::ImageSubresourceRange{
                                                                                vk::ImageAspectFlagBits::eColor, 0, 1,
                                                                                0, 1}};

    static inline vk::ImageCreateInfo defaultDepthCreateInfo = {vk::ImageCreateFlags(),
                                                                vk::ImageType::e2D, vk::Format::eD32Sfloat,
                                                                vk::Extent3D{800, 600, 1},
                                                                1, 1, vk::SampleCountFlagBits::e1,
                                                                vk::ImageTiling::eOptimal,
                                                                vk::ImageUsageFlagBits::eDepthStencilAttachment |
                                                                vk::ImageUsageFlagBits::eSampled |
                                                                vk::ImageUsageFlagBits::eTransferSrc,
                                                                vk::SharingMode::eExclusive,
                                                                0, 0,
                                                                vk::ImageLayout::eUndefined, nullptr

    };
    static inline vk::ImageViewCreateInfo defaultDepthViewInfo = {vk::ImageViewCreateFlags(),
                                                                  nullptr, vk::ImageViewType::e2D,
                                                                  vk::Format::eD32Sfloat, {},
                                                                  vk::ImageSubresourceRange{
                                                                          vk::ImageAspectFlagBits::eDepth, 0, 1,
                                                                          0, 1}};
public:
    FrameBufferManager(std::shared_ptr<LogicalDevice> device,
                       uint32_t attachmentPerRenderStepAmount,
                       uint32_t maxFramesInFlight, uint32_t width, uint32_t height) : device(device),
                                                                                      attachmentPerStepAmount(
                                                                                              attachmentPerRenderStepAmount),
                                                                                      width(width), height(height),
                                                                                      maxFramesInFlight(
                                                                                              maxFramesInFlight),
                                                                                      swapChain(nullptr) {
        createAttachments();
    }

    FrameBufferManager(std::shared_ptr<LogicalDevice> device, std::shared_ptr<SwapChain> swapChain,
                       uint32_t maxFramesInFlight, uint32_t width, uint32_t height) : device(device),
                                                                                      swapChain(std::move(swapChain)),
                                                                                      maxFramesInFlight(
                                                                                              maxFramesInFlight),
                                                                                      width(width), height(height),
                                                                                      attachmentPerStepAmount(1) {
        createAttachmentsFromSwapChain();
    }


private:
    std::vector<std::shared_ptr<Image>> renderImages;
    std::vector<std::shared_ptr<ImageView>> attachments;
    std::shared_ptr<LogicalDevice> device;
    uint32_t attachmentPerStepAmount;
    uint32_t maxFramesInFlight;
    std::vector<std::shared_ptr<FrameBuffer>> frameBuffers;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<SwapChain> swapChain;
    uint32_t width;
    uint32_t height;
public:
    void resize(uint32_t width, uint32_t height) {
        frameBuffers.clear();
        attachments.clear();
        if (swapChain != nullptr) {
            swapChain->recreate(width, height, false);
            resizeAttachmentsFromSwapChain(width, height);
        } else {
            resizeAttachments(width, height);
        }
        createFrameBuffers();
    }

    std::shared_ptr<FrameBuffer> getFrameBuffer(uint32_t imageIndex) {
        return frameBuffers[imageIndex];
    }

    std::shared_ptr<ImageView> *getColorAttachments() {
        return &attachments[0];
    }

    std::shared_ptr<ImageView> *getDepthAttachments() {
        return &attachments[attachmentPerStepAmount];
    }

    void initializeFrameBuffers(std::shared_ptr<RenderPass> renderPass) {
        FrameBufferManager::renderPass = renderPass;
        createFrameBuffers();
    }

private:
    void createFrameBuffers() {
        for (uint32_t i = 0; i < attachments.size(); i += attachmentPerStepAmount + 1) {
            frameBuffers.push_back(std::make_shared<FrameBuffer>(
                    device, renderPass->getRenderPass(), &attachments[i],
                    attachmentPerStepAmount + 1, width, height));
        }
    }

    void createAttachmentsFromSwapChain() {
        vk::Format depthFormat = device->findDepthFormat();
        defaultColorCreateInfo.extent = vk::Extent3D{width, height, 1};
        defaultDepthCreateInfo.extent = vk::Extent3D{width, height, 1};
        defaultDepthCreateInfo.format = depthFormat;
        defaultDepthViewInfo.format = depthFormat;
        for (auto &item: swapChain->getSwapchainImageViews()) {
            attachments.push_back(item);
            renderImages.push_back(std::make_shared<Image>(device, defaultDepthCreateInfo));
            defaultDepthViewInfo.image = renderImages[renderImages.size() - 1]->getBase();
            attachments.push_back(renderImages[renderImages.size() - 1]->createImageView(defaultDepthViewInfo));
        }
    }

    void resizeAttachmentsFromSwapChain(uint32_t width, uint32_t height) {
        FrameBufferManager::width = width;
        FrameBufferManager::height = height;
        vk::Format depthFormat = device->findDepthFormat();
        defaultDepthViewInfo.format = depthFormat;
        uint32_t i = 0;
        for (auto &item: swapChain->getSwapchainImageViews()) {
            attachments.push_back(item);
            renderImages[i]->resize(width, height);
            attachments.push_back(renderImages[i]->getImageViews()[0]);
            i++;
        }
    }

    void resizeAttachments(uint32_t width, uint32_t height) {
        FrameBufferManager::width = width;
        FrameBufferManager::height = height;
        vk::Format depthFormat = device->findDepthFormat();
        defaultDepthViewInfo.format = depthFormat;
        for (uint32_t i = 0; i < (attachmentPerStepAmount + 1) * maxFramesInFlight; i++) {
            renderImages[i]->resize(width, height);
            attachments.push_back(renderImages[renderImages.size() - 1]->getImageViews()[0]);


        }
    }

    void createAttachments() {
        vk::Format depthFormat = device->findDepthFormat();
        defaultColorCreateInfo.extent = vk::Extent3D{width, height, 1};
        defaultDepthCreateInfo.extent = vk::Extent3D{width, height, 1};
        defaultDepthCreateInfo.format = depthFormat;
        defaultDepthViewInfo.format = depthFormat;
        for (uint32_t i = 0;
             i < attachmentPerStepAmount * maxFramesInFlight; i += attachmentPerStepAmount) {
            for (uint32_t ii = i; ii < i + attachmentPerStepAmount; ii++) {
                renderImages.push_back(std::make_shared<Image>(device, defaultColorCreateInfo));
                defaultColorViewCreateInfo.image = renderImages.end()->get()->getBase();
                attachments.push_back(renderImages.end()->get()->createImageView(defaultColorViewCreateInfo));
            }
            defaultDepthViewInfo.image = renderImages.end()->get()->getBase();
            attachments.push_back(renderImages.end()->get()->createImageView(defaultDepthViewInfo));
        }
    }

public:
    void destroy() override {
        destroyed = true;
        frameBuffers.clear();
        attachments.clear();
        renderImages.clear();
    }
};


