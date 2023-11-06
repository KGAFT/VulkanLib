//
// Created by kgaft on 11/5/23.
//
#pragma once

#include "VulkanLib/Device/PhysicalDevice/DeviceSuitability.hpp"
#include "VulkanLib/MemoryUtils/MemoryUtils.hpp"
#include "LogicalQueue.hpp"

class LogicalDevice {
private:
    static inline std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();
    static inline float priority = 1.0f;
    static inline unsigned int usedQueueCreateInfos;
public:
    LogicalDevice(Instance& instance, PhysicalDevice* device, DeviceBuilder &builder, DeviceSuitabilityResults *results) :baseDevice(device) {
        sanitizeQueueCreateInfos(results);

        vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
                vk::DeviceCreateFlags(),
                usedQueueCreateInfos, queueCreateInfos.data(),
                instance.getEnabledLayers().size(), instance.getEnabledLayers().data(),
                builder.requestExtensions.size(), builder.requestExtensions.data(),
                nullptr
        );
        try{
            LogicalDevice::device = device->getBase().createDevice(deviceInfo, nullptr);
            for (const auto &item: results->queuesInfo){
                queues.push_back(LogicalQueue(LogicalDevice::device.getQueue(item.index, 0), item.supportPresentation, item.properties.queueFlags&vk::QueueFlagBits::eGraphics?vk::QueueFlagBits::eGraphics:vk::QueueFlagBits::eCompute));
            }
        }catch (vk::SystemError& error){
            std::cerr<<error.what()<<std::endl;
        }


    }
private:
    vk::Device device;
    std::vector<LogicalQueue> queues;
    PhysicalDevice* baseDevice;
public:
    const vk::Device &getDevice() const {
        device;
    }

    PhysicalDevice *getBaseDevice() const {
        return baseDevice;
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

public:
    ~LogicalDevice() {

        device.destroy();
    }

};


