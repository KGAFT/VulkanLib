//
// Created by kgaft on 11/5/23.
//
#pragma once

#include "VulkanLib/Device/PhysicalDevice/DeviceSuitability.hpp"
#include "VulkanLib/MemoryUtils/MemoryUtils.hpp"
#include "LogicalQueue.hpp"

class LogicalDevice : IDestroyableObject {
private:
    static inline std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();
    static inline float priority = 1.0f;
    static inline unsigned int usedQueueCreateInfos;
public:
    LogicalDevice(Instance &instance, PhysicalDevice *device, DeviceBuilder &builder, DeviceSuitabilityResults *results)
            : baseDevice(device) {
        sanitizeQueueCreateInfos(results);

        vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
                vk::DeviceCreateFlags(),
                usedQueueCreateInfos, queueCreateInfos.data(),
                instance.getEnabledLayers().size(), instance.getEnabledLayers().data(),
                builder.requestExtensions.size(), builder.requestExtensions.data(),
                nullptr
        );
        try {
            LogicalDevice::device = device->getBase().createDevice(deviceInfo, nullptr);
            for (const auto &item: results->queuesInfo) {
                queues.push_back(LogicalQueue(LogicalDevice::device.getQueue(item.index, 0), item.supportPresentation,
                                              item.properties.queueFlags & vk::QueueFlagBits::eGraphics
                                              ? vk::QueueFlagBits::eGraphics : vk::QueueFlagBits::eCompute,
                                              item.index));
            }
        } catch (vk::SystemError &error) {
            std::cerr << error.what() << std::endl;
        }


    }

    LogicalDevice() {

    }

private:
    vk::Device device;
    std::vector<LogicalQueue> queues;
    PhysicalDevice *baseDevice;
public:
    const vk::Device &getDevice() const {
        return device;
    }

    PhysicalDevice *getBaseDevice() const {
        return baseDevice;
    }

    LogicalQueue &getQueueByType(vk::QueueFlagBits queueType) {
        for (auto &item: queues) {
            if (item.queueType & queueType) {
                return item;
            }
        }
        throw std::runtime_error("Error: no such queue");
    }

    LogicalQueue &getPresentQueue() {
        for (auto &item: queues) {
            if (item.supportPresentation) {
                return item;
            }
        }
        throw std::runtime_error("Error: no such queue");
    }

    vk::Format findDepthFormat() {
        return findSupportedFormat(
                {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                vk::ImageTiling::eOptimal,
                vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }

    unsigned int findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties;
        baseDevice->getBase().getMemoryProperties(&memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
    vk::Format findSupportedFormat(
            const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for (vk::Format format: candidates) {
            vk::FormatProperties props;
            baseDevice->getBase().getFormatProperties(format, &props);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (
                    tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    unsigned int getQueuesAmount() {
        return queues.size();
    }

private:
    void sanitizeQueueCreateInfos(DeviceSuitabilityResults *results) {
        unsigned int counter = 0;
        for (const auto &item: results->queuesInfo) {
            if (counter >= queueCreateInfos.size()) {
                vk::DeviceQueueCreateInfo queueCreateInfo(
                        vk::DeviceQueueCreateFlags(), item.index,
                        1, &priority);
                queueCreateInfos.push_back(queueCreateInfo);
            } else {
                MemoryUtils::memClear(&queueCreateInfos[counter], sizeof(vk::DeviceQueueCreateInfo));
                new(&queueCreateInfos[counter]) vk::DeviceQueueCreateInfo(
                        vk::DeviceQueueCreateFlags(), item.index,
                        1, &priority);
            }
            counter++;
        }
        usedQueueCreateInfos = counter;
    }

    void destroy() override {
        device.destroy();
        destroyed = true;
    }


};


