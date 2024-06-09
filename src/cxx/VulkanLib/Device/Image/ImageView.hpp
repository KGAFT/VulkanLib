//
// Created by kgaft on 11/7/23.
//
#ifndef VULKANLIB_IMAGEVIEW_HPP
#define VULKANLIB_IMAGEVIEW_HPP

#include <vulkan/vulkan.hpp>
#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>

class ImageView : IDestroyableObject {
    friend class Image;
public:
    ImageView(vk::ImageCreateInfo &parentInfo, std::shared_ptr<LogicalDevice> device, vk::ImageView base,
              vk::ImageViewCreateInfo &createInfo);

private:
    vk::ImageCreateInfo &parentInfo;
    std::shared_ptr<LogicalDevice> device;
    vk::ImageView base;
    vk::ImageViewCreateInfo createInfo;
public:
    const vk::ImageView &getBase() const;

    const vk::ImageViewCreateInfo &getCreateInfo() const;

    vk::ImageCreateInfo &getParentInfo() const;

public:
    void destroy() override;

    virtual ~ImageView() = default;
};
#endif

