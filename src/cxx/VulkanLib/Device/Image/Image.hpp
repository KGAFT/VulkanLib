//
// Created by kgaft on 11/7/23.
//
#ifndef VULKANLIB_IMAGE_HPP
#define VULKANLIB_IMAGE_HPP
#include "ImageView.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/Device/Buffer/Buffer.hpp"
#include <memory>
#include <vulkan/vulkan_structs.hpp>

class Image : public IDestroyableObject {
public:
    Image(std::shared_ptr<LogicalDevice> device, vk::Image base);

    Image(std::shared_ptr<LogicalDevice>& device, vk::ImageCreateInfo &createInfo);

    Image();

private:
    std::shared_ptr<LogicalDevice> device;
    vk::Image base;
    vk::ImageCreateInfo imageInfo;
    std::vector<std::shared_ptr<ImageView>> imageViews;
    vk::DeviceMemory imageMemory;
    bool castCreated = false;
public:
    std::shared_ptr<ImageView> createImageView(vk::ImageViewCreateInfo &createInfo);

    void
    initialize(std::shared_ptr<LogicalDevice> device, vk::Image base);

    void initialize(std::shared_ptr<LogicalDevice>& device, vk::ImageCreateInfo &createInfo);

    vk::Image &getBase();

    void copyFromBuffer(Buffer& buffer, uint32_t layerCount, LogicalQueue& queue);

    void transitionImageLayout(std::shared_ptr<LogicalDevice> device, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags);

    void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags);

    std::vector<std::shared_ptr<ImageView>> &getImageViews();

    void resize(uint32_t width, uint32_t height);

    vk::ImageCreateInfo &getImageInfo();

public:
    void destroy() override;

};

#endif