//
// Created by kgaft on 11/6/23.
//
#pragma once

#include "VulkanLib/Device/Image/Image.hpp"
#include <VulkanLib/Device/LogicalDevice/LogicalDevice.hpp>
#include <VulkanLib/MemoryUtils/VectorUtils.hpp>
#include <utility>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class SwapChain : public IDestroyableObject {
private:
  static inline vk::SurfaceFormatKHR format = {};

public:
  static const vk::SurfaceFormatKHR &getFormat() { return format; }

public:
  SwapChain(std::shared_ptr<LogicalDevice> device,
            const vk::SurfaceKHR &surface, uint32_t width, uint32_t height,
            bool enableFrameLock)
      : device(std::move(device)), surface(surface), width(width),
        height(height), enableFrameLock(enableFrameLock) {
    createSwapChain(width, height, enableFrameLock);
  }

private:
  std::shared_ptr<LogicalDevice> device;
  vk::SwapchainKHR swapchainKhr = vk::SwapchainKHR{nullptr};
  vk::SurfaceKHR surface;
  vk::PresentModeKHR presentMode;
  vk::Extent2D extent;
  std::vector<vk::Image> baseImages;
  std::vector<std::shared_ptr<Image>> swapchainImages;
  std::vector<std::shared_ptr<ImageView>> swapchainImageViews;
  uint32_t width;
  uint32_t height;
  bool enableFrameLock = false;

public:
  const std::vector<std::shared_ptr<ImageView>> &
  getSwapchainImageViews() const {
    return swapchainImageViews;
  }

  const vk::SwapchainKHR &getSwapchainKhr() const { return swapchainKhr; }
  void recreate(uint32_t width, uint32_t height) {
    cleanUpImages();
    createSwapChain(width, height, enableFrameLock);
  }

  void recreate(uint32_t width, uint32_t height, bool refreshRateLock) {
    cleanUpImages();
    enableFrameLock = refreshRateLock;
    createSwapChain(width, height, refreshRateLock);
  }

  const std::vector<std::shared_ptr<Image>> &getSwapchainImages() const {
    return swapchainImages;
  }

private:
  void createSwapChain(uint32_t width, uint32_t height, bool enableFrameLock) {
    SwapChainSupportDetails support{};
    MemoryUtils::memClear(&support, sizeof(SwapChainSupportDetails));
    gatherSwapChainSupportDetails(support);

    format = chooseSurfaceFormat(support.formats);
    presentMode = choosePresentMode(support.presentModes, enableFrameLock);
    extent = chooseSwapchainExtent(width, height, support.capabilities);
    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 &&
        imageCount > support.capabilities.maxImageCount) {
      imageCount = support.capabilities.maxImageCount;
    }
    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(), surface, imageCount, format.format,
        format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);
    std::vector<unsigned int> queueIndices = {
        device->getQueueByType(vk::QueueFlagBits::eGraphics)->getIndex(),
        device->getPresentQueue()->getIndex()};
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

    createInfo.oldSwapchain = swapchainKhr;
    try {
      swapchainKhr = device->getDevice().createSwapchainKHR(createInfo);
      baseImages = device->getDevice().getSwapchainImagesKHR(swapchainKhr);
      swapchainImages.resize(baseImages.size());
      for (uint32_t i = 0; i < baseImages.size(); ++i) {
        swapchainImages[i] =
            std::shared_ptr<Image>(new Image(device, baseImages[i]));
        vk::ImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.image = baseImages[i];
        viewCreateInfo.viewType = vk::ImageViewType::e2D;
        viewCreateInfo.format = format.format;
        viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.subresourceRange.aspectMask =
            vk::ImageAspectFlagBits::eColor;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        swapchainImageViews.push_back(
            swapchainImages[i]->createImageView(viewCreateInfo));
      }
    } catch (vk::SystemError &err) {
      std::cerr << err.what() << std::endl;
    }
  }

  void gatherSwapChainSupportDetails(SwapChainSupportDetails &output) {
    vk::Result res;
    res = device->getBaseDevice()->getBase().getSurfaceCapabilitiesKHR(
        surface, &output.capabilities);
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    uint32_t formatCount;
    res = device->getBaseDevice()->getBase().getSurfaceFormatsKHR(
        surface, &formatCount, nullptr);
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    output.formats.resize(formatCount);
    res = device->getBaseDevice()->getBase().getSurfaceFormatsKHR(
        surface, &formatCount, output.formats.data());
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    res = device->getBaseDevice()->getBase().getSurfacePresentModesKHR(
        surface, &formatCount, nullptr);
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    output.presentModes.resize(formatCount);
    res = device->getBaseDevice()->getBase().getSurfacePresentModesKHR(
        surface, &formatCount, output.presentModes.data());
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
  }

  vk::SurfaceFormatKHR
  chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> &formats) {
    std::vector<vk::Format> preferredFormats = {
        vk::Format::eR64G64B64A64Sfloat, vk::Format::eR32G32B32A32Sfloat,
        vk::Format::eR16G16B16A16Sfloat, vk::Format::eB8G8R8A8Srgb};
    vk::SurfaceFormatKHR selectedFormat{};
    selectedFormat.format = static_cast<vk::Format>(0);
    selectedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    for (const auto &item : formats) {
      for (const auto &citem : preferredFormats) {
        if (item.format == citem &&
            (item.format > selectedFormat.format ||
             (item.colorSpace >= selectedFormat.colorSpace &&
              item.format >= selectedFormat.format))) {
          selectedFormat = item;
        }
      }
    }
    if (static_cast<int>(selectedFormat.format) != 0) {
      return selectedFormat;
    }
    return formats[0];
  }

  vk::PresentModeKHR
  choosePresentMode(std::vector<vk::PresentModeKHR> &presentModes,
                    bool enableFrameLock) {
    for (vk::PresentModeKHR presentMode : presentModes) {
      if (presentMode == (enableFrameLock ? vk::PresentModeKHR::eMailbox
                                          : vk::PresentModeKHR::eImmediate)) {
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
  void cleanUpImages() {
    for (const auto &item : swapchainImageViews) {
      item->destroy();
    }
    swapchainImageViews.clear();
    swapchainImages.clear();
  }

public:
  void destroy() override {
    cleanUpImages();
    device->getDevice().destroySwapchainKHR(swapchainKhr);
    swapchainKhr = vk::SwapchainKHR{nullptr};
    destroyed = true;
  }
};
