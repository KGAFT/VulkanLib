//
// Created by kgaft on 11/6/23.
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain {
private:
    LogicalDevice device;
    vk::SwapchainKHR swapchainKhr;
    vk::SurfaceKHR surface;
private:
    void gatherSwapChainSupportDetails(SwapChainSupportDetails& output){
        device.getBaseDevice()->getBase().getSurfaceCapabilitiesKHR(surface, &output.capabilities);
        uint32_t formatCount;
        device.getBaseDevice()->getBase().getSurfaceFormatsKHR(surface, &formatCount, nullptr);
        output.formats.resize(formatCount);
        device.getBaseDevice()->getBase().getSurfaceFormatsKHR(surface, &formatCount, output.formats.data());
        device.getBaseDevice()->getBase().getSurfacePresentModesKHR(surface, &formatCount, nullptr);
        output.presentModes.resize(formatCount);
        device.getBaseDevice()->getBase().getSurfacePresentModesKHR(surface, &formatCount, output.presentModes.data());
    }
};


