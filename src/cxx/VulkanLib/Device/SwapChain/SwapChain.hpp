//
// Created by kgaft on 11/6/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
#include <VulkanLib/MemoryUtils/VectorUtils.hpp>
#include <set>

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain {
public:
    SwapChain(const LogicalDevice &device, const vk::SurfaceKHR &surface, uint32_t width, uint32_t height) : device(device), surface(surface), width(width), height(height) {
        createSwapChain(width, height);
    }

private:
    LogicalDevice device;
    vk::SwapchainKHR swapchainKhr;
    vk::SurfaceKHR surface;
    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentMode;
    vk::Extent2D extent;
    std::vector<vk::Image> swapchainImages;
    uint32_t width;
    uint32_t height;
private:
    void createSwapChain(uint32_t width, uint32_t height) {
        SwapChainSupportDetails support{};
        MemoryUtils::memClear(&support, sizeof(SwapChainSupportDetails));
        gatherSwapChainSupportDetails(support);

        format = chooseSurfaceFormat(support.formats);
        presentMode = choosePresentMode(support.presentModes);
        extent = chooseSwapchainExtent(width, height, support.capabilities);

        uint32_t imageCount = std::min(
                support.capabilities.maxImageCount,
                support.capabilities.minImageCount + 1
        );
        vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
                vk::SwapchainCreateFlagsKHR(), surface, imageCount, format.format, format.colorSpace,
                extent, 1, vk::ImageUsageFlagBits::eColorAttachment
        );
        std::vector<unsigned int> queueIndices = {device.getQueueByType(vk::QueueFlagBits::eGraphics).getIndex(),
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
            uint32_t imageAmount;
            device.getDevice().getSwapchainImagesKHR(swapchainKhr, &imageAmount, nullptr);
            swapchainImages.resize(imageAmount);
            device.getDevice().getSwapchainImagesKHR(swapchainKhr, &imageAmount, swapchainImages.data());

        } catch (vk::SystemError &err) {
            std::cerr << err.what() << std::endl;
        }
    }

    void gatherSwapChainSupportDetails(SwapChainSupportDetails &output) {
        device.getBaseDevice()->getBase().getSurfaceCapabilitiesKHR(surface, &output.capabilities);
        uint32_t formatCount;
        device.getBaseDevice()->getBase().getSurfaceFormatsKHR(surface, &formatCount, nullptr);
        output.formats.resize(formatCount);
        device.getBaseDevice()->getBase().getSurfaceFormatsKHR(surface, &formatCount, output.formats.data());
        device.getBaseDevice()->getBase().getSurfacePresentModesKHR(surface, &formatCount, nullptr);
        output.presentModes.resize(formatCount);
        device.getBaseDevice()->getBase().getSurfacePresentModesKHR(surface, &formatCount, output.presentModes.data());
    }


    vk::SurfaceFormatKHR chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> &formats) {

        for (vk::SurfaceFormatKHR format: formats) {
            if (format.format == vk::Format::eB8G8R8A8Unorm
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }

        return formats[0];
    }

    vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR> &presentModes) {
        for (vk::PresentModeKHR presentMode: presentModes) {
            if (presentMode == vk::PresentModeKHR::eMailbox) {
                return presentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapchainExtent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR &capabilities) {

        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            vk::Extent2D extent = {width, height};

            extent.width = std::min(
                    capabilities.maxImageExtent.width,
                    std::max(capabilities.minImageExtent.width, extent.width)
            );

            extent.height = std::min(
                    capabilities.maxImageExtent.height,
                    std::max(capabilities.minImageExtent.height, extent.height)
            );

            return extent;
        }
    }
};


