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
        base = device.getDevice().createImage(createInfo);
    }
    Image(){
        
    }

private:
    LogicalDevice *device;
    vk::Image base;
    vk::ImageCreateInfo imageInfo;
    std::vector<ImageView> imageViews;
    bool castCreated = false;
public:
    ImageView &createImageView(vk::ImageViewCreateInfo &createInfo) {
        imageViews.push_back(ImageView(
                *device, device->getDevice().createImageView(createInfo), createInfo));
        return imageViews[imageViews.size() - 1];
    }
    void initialize(LogicalDevice *device, vk::Image base) {
        Image::device = device;
        Image::base = base;
    }
    const std::vector<ImageView> &getImageViews() const { return imageViews; }

private:
    void destroy() override {
        device->getDevice().destroyImage(base);
        destroyed = true;
    }

};