//
// Created by kgaft on 11/6/23.
//
#pragma once

#include "VulkanLib/Device/Image/Image.hpp"
#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
#include <VulkanLib/MemoryUtils/VectorUtils.hpp>
#include <vulkan/vulkan.hpp>

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain : IDestroyableObject {
private:
    static inline vk::SurfaceFormatKHR format = {};
public:
    static const vk::SurfaceFormatKHR &getFormat() {
        return format;
    }

public:
    SwapChain(LogicalDevice &device, const vk::SurfaceKHR &surface,
              uint32_t width, uint32_t height)
            : device(device), surface(surface), width(width), height(height) {
        createSwapChain(width, height);
    }

private:
    LogicalDevice &device;
    vk::SwapchainKHR swapchainKhr;
    vk::SurfaceKHR surface;
    vk::PresentModeKHR presentMode;
    vk::Extent2D extent;
    std::vector<Image> swapchainImages;
    std::vector<ImageView *> swapchainImageViews;
    uint32_t width;
    uint32_t height;
public:
    const std::vector<ImageView *> &getSwapchainImageViews() const {
        return swapchainImageViews;
    }

private:
    void createSwapChain(uint32_t width, uint32_t height) {
        SwapChainSupportDetails support{};
        MemoryUtils::memClear(&support, sizeof(SwapChainSupportDetails));
        gatherSwapChainSupportDetails(support);

        format = chooseSurfaceFormat(support.formats);
        presentMode = choosePresentMode(support.presentModes);
        extent = chooseSwapchainExtent(width, height, support.capabilities);

        uint32_t imageCount = std::min(support.capabilities.maxImageCount,
                                       support.capabilities.minImageCount + 1);
        vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
                vk::SwapchainCreateFlagsKHR(), surface, imageCount, format.format,
                format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);
        std::vector<unsigned int> queueIndices = {
                device.getQueueByType(vk::QueueFlagBits::eGraphics).getIndex(),
                device.getPresentQueue().getIndex()};
        VectorUtils::removeRepeatingElements(queueIndices);
        if (queueIndices.size() > 1) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = queueIndices.size();
            createInfo.pQueueFamilyIndices = queueIndices.data();
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }
        createInfo.preTransform = support.capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);
        try {
            swapchainKhr = device.getDevice().createSwapchainKHR(createInfo);
            std::vector<vk::Image> images = device.getDevice().getSwapchainImagesKHR(swapchainKhr);
            swapchainImages.resize(images.size());
            for (int i = 0; i < images.size(); ++i) {
                swapchainImages[i].initialize(&device, images[i]);
                vk::ImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.image = images[i];
                viewCreateInfo.viewType = vk::ImageViewType::e2D;
                viewCreateInfo.format = format.format;
                viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;
                swapchainImageViews.push_back(&swapchainImages[i].createImageView(viewCreateInfo));

            }
        } catch (vk::SystemError &err) {
            std::cerr << err.what() << std::endl;
        }
    }

    void gatherSwapChainSupportDetails(SwapChainSupportDetails &output) {
        vk::Result res;
        res = device.getBaseDevice()->getBase().getSurfaceCapabilitiesKHR(
                surface, &output.capabilities);
        uint32_t formatCount;
        res = device.getBaseDevice()->getBase().getSurfaceFormatsKHR(
                surface, &formatCount, nullptr);
        output.formats.resize(formatCount);
        res = device.getBaseDevice()->getBase().getSurfaceFormatsKHR(
                surface, &formatCount, output.formats.data());
        res = device.getBaseDevice()->getBase().getSurfacePresentModesKHR(
                surface, &formatCount, nullptr);
        output.presentModes.resize(formatCount);
        res = device.getBaseDevice()->getBase().getSurfacePresentModesKHR(
                surface, &formatCount, output.presentModes.data());
    }

    vk::SurfaceFormatKHR
    chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> &formats) {

        for (vk::SurfaceFormatKHR format: formats) {
            if (format.format == vk::Format::eB8G8R8A8Unorm &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }

        return formats[0];
    }

    vk::PresentModeKHR
    choosePresentMode(std::vector<vk::PresentModeKHR> &presentModes) {
        for (vk::PresentModeKHR presentMode: presentModes) {
            if (presentMode == vk::PresentModeKHR::eMailbox) {
                return presentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapchainExtent(uint32_t width, uint32_t height,
                                       vk::SurfaceCapabilitiesKHR &capabilities) {

        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            vk::Extent2D extent = {width, height};

            extent.width =
                    std::min(capabilities.maxImageExtent.width,
                             std::max(capabilities.minImageExtent.width, extent.width));

            extent.height =
                    std::min(capabilities.maxImageExtent.height,
                             std::max(capabilities.minImageExtent.height, extent.height));

            return extent;
        }
    }

    virtual void destroy() override {
        device.getDevice().destroySwapchainKHR(swapchainKhr);
        destroyed = true;
    }

};
