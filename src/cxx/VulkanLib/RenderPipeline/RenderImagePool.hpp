//
// Created by kgaft on 12/21/23.
//
#ifndef VULKANLIB_RENDERIMAGEPOOL_HPP
#define VULKANLIB_RENDERIMAGEPOOL_HPP

#include "VulkanLib/Device/Image/Image.hpp"
#include <map>
#include <memory>

class RenderImagePool : public IDestroyableObject {


public:
    RenderImagePool(std::shared_ptr<LogicalDevice> device) : device(device){

    }
    RenderImagePool(){

    }

private:
    std::map<std::shared_ptr<Image>, bool> createdColorImages;
    std::map<std::shared_ptr<Image>, bool> createdDepthImages;
    std::shared_ptr<LogicalDevice> device;
public:
    std::shared_ptr<Image> acquireDepthImage(uint32_t width, uint32_t height) {
        for (auto &item: createdDepthImages) {
            if (!item.second) {
                item.first->resize(width, height);
                item.second = true;
                return item.first;
            }
        }
        auto result = createDepthAttachment(device, width, height);
        createdDepthImages.insert(std::pair(result, true));
        return result;
    }

    void releaseDepthImage(std::shared_ptr<Image> image) {
        for (auto &item: createdDepthImages) {
            if (item.first == image) {
                item.second = false;
                break;
            }
        }
    }


    std::shared_ptr<Image>
    acquireColorRenderImage(uint32_t width, uint32_t height) {
        for (auto &item: createdColorImages) {
            if (!item.second) {
                item.first->resize(width, height);
                item.second = true;
                return item.first;
            }
        }
        auto result = createColorAttachment(device, width, height);
        createdColorImages.insert(std::pair(result, true));
        return result;
    }

    void releaseColorImage(std::shared_ptr<Image> image) {
        for (auto &item: createdColorImages) {
            if (item.first == image) {
                item.second = false;
                break;
            }
        }
    }

public:
    void destroy() override {
        destroyed  = true;
        for (auto &item: createdColorImages){
            item.first->destroy();
        }
        for (auto &item: createdDepthImages){
            item.first->destroy();
        }
    }

private:
    /*
     * @return image that can be used as depth attachment. View for framebuffer available at first index of image views
     */
    static std::shared_ptr<Image>
    createDepthAttachment(std::shared_ptr<LogicalDevice> device, uint32_t width, uint32_t height) {
        vk::Format depthFormat = device->findDepthFormat();
        defaultDepthCreateInfo.extent = vk::Extent3D{width, height, 1};
        defaultDepthCreateInfo.format = depthFormat;
        defaultDepthViewInfo.format = depthFormat;
        auto result = std::make_shared<Image>(device, defaultDepthCreateInfo);
        defaultDepthViewInfo.image = result->getBase();
        result->createImageView(defaultDepthViewInfo);
        result->transitionImageLayout(device, vk::ImageLayout::eUndefined,
                                          vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                          vk::ImageAspectFlagBits::eDepth);
        return result;
    }

    /*
     * @return image that can be used as color attachment. View for framebuffer available at first index of image views
     */
    static std::shared_ptr<Image>
    createColorAttachment(std::shared_ptr<LogicalDevice> device, uint32_t width, uint32_t height) {
        defaultColorCreateInfo.extent = vk::Extent3D{width, height, 1};
        auto result = std::make_shared<Image>(device, defaultColorCreateInfo);
        defaultColorViewCreateInfo.image = result->getBase();
        result->createImageView(defaultColorViewCreateInfo);
        result->transitionImageLayout(device, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, vk::ImageAspectFlagBits::eColor);
        result->getImageInfo().initialLayout = vk::ImageLayout::eGeneral;
        return result;
    }

private:
    static inline vk::ImageCreateInfo defaultColorCreateInfo = {vk::ImageCreateFlags(),
                                                                vk::ImageType::e2D, vk::Format::eR32G32B32A32Sfloat,
                                                                vk::Extent3D{800, 600, 1},
                                                                1, 1, vk::SampleCountFlagBits::e1,
                                                                vk::ImageTiling::eOptimal,
                                                                vk::ImageUsageFlagBits::eColorAttachment |
                                                                vk::ImageUsageFlagBits::eSampled |
                                                                vk::ImageUsageFlagBits::eTransferSrc,
                                                                vk::SharingMode::eExclusive,
                                                                0, nullptr,
                                                                vk::ImageLayout::eUndefined, nullptr

    };
    static inline vk::ImageViewCreateInfo defaultColorViewCreateInfo = {vk::ImageViewCreateFlags(),
                                                                        nullptr, vk::ImageViewType::e2D,
                                                                        vk::Format::eR32G32B32A32Sfloat, {},
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
};

#endif

