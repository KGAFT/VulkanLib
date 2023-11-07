//
// Created by kgaft on 11/7/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
class ImageView : IDestroyableObject{
public:
    ImageView(LogicalDevice& device, vk::ImageView base, vk::ImageViewCreateInfo &createInfo) : base(base),
                                                                                      createInfo(createInfo), device(device) {}

private:
    LogicalDevice& device;
    vk::ImageView base;
    vk::ImageViewCreateInfo createInfo;
public:
    const vk::ImageView &getBase() const {
        return base;
    }

    const vk::ImageViewCreateInfo &getCreateInfo() const {
        return createInfo;
    }

private:
    void destroy() override {
        device.getDevice().destroyImageView(base);
        destroyed = true;
    }


};


