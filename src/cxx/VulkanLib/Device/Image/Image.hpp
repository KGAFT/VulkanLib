//
// Created by kgaft on 11/7/23.
//
#pragma once

#include "ImageView.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include <memory>
#include <vulkan/vulkan_structs.hpp>

class Image : IDestroyableObject {
public:
    Image(std::shared_ptr<LogicalDevice> device, vk::Image base) : device(std::move(device)), base(base) {
        castCreated = true;
    }

    Image(std::shared_ptr<LogicalDevice>& device, vk::ImageCreateInfo &createInfo) : imageInfo(createInfo) {
        initialize(device, createInfo);
    }

    Image() {

    }

private:
    std::shared_ptr<LogicalDevice> device;
    vk::Image base;
    vk::ImageCreateInfo imageInfo;
    std::vector<std::shared_ptr<ImageView>> imageViews;
    vk::DeviceMemory imageMemory;
    bool castCreated = false;
public:
    std::shared_ptr<ImageView> createImageView(vk::ImageViewCreateInfo &createInfo) {
        vk::ImageView view = Image::device->getDevice().createImageView(createInfo);
        imageViews.push_back(std::make_shared<ImageView>(imageInfo,
                                                         Image::device, view, createInfo));
        return imageViews[imageViews.size() - 1];
    }

    void
    initialize(std::shared_ptr<LogicalDevice> device, vk::Image base) {
        Image::device = device;
        Image::base = base;
    }

    void initialize(std::shared_ptr<LogicalDevice>& device, vk::ImageCreateInfo &createInfo) {
        vk::Result res;
        Image::device = device;
        Image::imageInfo = createInfo;
        base =  Image::device->getDevice().createImage(createInfo);

        vk::MemoryRequirements requirements;
        Image::device->getDevice().getImageMemoryRequirements(base, &requirements);

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex =  Image::device->findMemoryType(requirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eDeviceLocal);

        res =  Image::device->getDevice().allocateMemory(&allocInfo, nullptr, &imageMemory);
        Image::device->getDevice().bindImageMemory(base, imageMemory, 0);

    }

    vk::Image &getBase() {
        return base;
    }

    std::vector<std::shared_ptr<ImageView>> &getImageViews() { return imageViews; }

    void resize(uint32_t width, uint32_t height) {
        if (!castCreated) {
            destroy();
            destroyed = false;
            imageInfo.extent = vk::Extent3D{width, height};
            initialize(device, imageInfo);
            for (auto &item: imageViews) {
                item->createInfo.image = base;
                item->base = device->getDevice().createImageView(item->createInfo);
                item->parentInfo = imageInfo;
            }
        }

    }

    vk::ImageCreateInfo &getImageInfo() {
        if (castCreated) {
            throw std::runtime_error("Error: you cannot get image info, from image created via cast");
        }
        return imageInfo;
    }

private:
    void destroy() override {
        if (!castCreated) {
            for (auto &item: imageViews) {
                item->destroy();
            }
            device->getDevice().destroyImage(base);

        }
        destroyed = true;
    }

};