//
// Created by kgaft on 11/5/23.
//
#pragma once

#include "VulkanLib/Device/PhysicalDevice/DeviceSuitability.hpp"
#include "VulkanLib/MemoryUtils/MemoryUtils.hpp"

class LogicalDevice {
private:
    static inline std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();
    static inline float priority = 1.0f;
    static inline unsigned int usedQueueCreateInfos;
public:
    LogicalDevice(Instance& instance, DeviceBuilder &builder, DeviceSuitabilityResults *results) {
        sanitizeQueueCreateInfos(results);

        vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
                vk::DeviceCreateFlags(),
                usedQueueCreateInfos, queueCreateInfos.data(),
                instance.getEnabledLayers().size(), instance.getEnabledLayers().data(),
                0, nullptr,
                nullptr
        );

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
};


