//
// Created by kgaft on 11/7/23.
//
#pragma once

#include "ImageView.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include <vulkan/vulkan_structs.hpp>

class Image : IDestroyableObject {
public:
    Image(LogicalDevice *&device, vk::Image base) : device(device), base(base) {
        castCreated = true;
    }

    Image(LogicalDevice &device, vk::ImageCreateInfo &createInfo) : imageInfo(createInfo), device(&device) {
        initialize(&device, createInfo);
    }

    Image() {

    }

private:
    LogicalDevice *device;
    vk::Image base;
    vk::ImageCreateInfo imageInfo;
    std::vector<ImageView> imageViews;
    vk::DeviceMemory imageMemory;
    bool castCreated = false;
public:
    ImageView &createImageView(vk::ImageViewCreateInfo &createInfo) {
        imageViews.push_back(ImageView(imageInfo,
                                       *device, device->getDevice().createImageView(createInfo), createInfo));
        return imageViews[imageViews.size() - 1];
    }

    void
    initialize(LogicalDevice *device, vk::Image base) {
        Image::device = device;
        Image::base = base;
    }

    void initialize(LogicalDevice *device, vk::ImageCreateInfo &createInfo) {
        Image::device = device;
        Image::imageInfo = createInfo;
        base = device->getDevice().createImage(createInfo);

        vk::MemoryRequirements requirements;
        device->getDevice().getImageMemoryRequirements(base, &requirements);

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(requirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eDeviceLocal);

        device->getDevice().allocateMemory(&allocInfo, nullptr, &imageMemory);
        device->getDevice().bindImageMemory(base, imageMemory, 0);

    }

    const vk::Image &getBase() const {
        return base;
    }

    [[nodiscard]] const std::vector<ImageView> &getImageViews() const { return imageViews; }

    const vk::ImageCreateInfo &getImageInfo() const {
        if (castCreated) {
            throw std::runtime_error("Error: you cannot get image info, from image created via cast");
        }
        return imageInfo;
    }

private:
    void destroy() override {
        if (!castCreated) {
            for (auto &item: imageViews) {
                item.destroy();
            }

            device->getDevice().destroyImage(base);

        }
        destroyed = true;
    }

};