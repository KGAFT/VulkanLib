//
// Created by kgaft on 3/11/24.
//
#include "LogicalDevice.hpp"

LogicalDevice::LogicalDevice(Instance &instance, std::shared_ptr<PhysicalDevice> device, DeviceBuilder &builder,
                             DeviceSuitabilityResults *results)
        : baseDevice(device) {
    sanitizeQueueCreateInfos(results);
    vk::PhysicalDeviceFeatures features{};
    features.shaderInt64 = true;
    vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature {};
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelStructure{};

    dynamicRenderingFeature.dynamicRendering = true;

    vk::PhysicalDeviceVulkan12Features newFeatures{};
    newFeatures.bufferDeviceAddress = true;
    newFeatures.descriptorIndexing = true;
    newFeatures.runtimeDescriptorArray = true;
    dynamicRenderingFeature.pNext = &newFeatures;
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKhr{};
    rayTracingPipelineFeaturesKhr.sType = vk::StructureType::ePhysicalDeviceRayTracingPipelineFeaturesKHR;
    rayTracingPipelineFeaturesKhr.rayTracingPipeline = true;
    if(builder.rayTracingSupport){
        accelStructure.accelerationStructure = true;
        newFeatures.pNext = &accelStructure;
        accelStructure.pNext = &rayTracingPipelineFeaturesKhr;
    }

    vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
            vk::DeviceCreateFlags(),
            usedQueueCreateInfos, queueCreateInfos.data(),
            instance.getEnabledLayers().size(), instance.getEnabledLayers().data(),
            builder.requestExtensions.size(), builder.requestExtensions.data(),
            &features
    );
    deviceInfo.pNext = &dynamicRenderingFeature;
    try {
        LogicalDevice::device = device->getBase().createDevice(deviceInfo, nullptr);
        for (const auto &item: results->queuesInfo) {
            queues.push_back(
                    std::make_shared<LogicalQueue>(LogicalDevice::device.getQueue(item.index, 0), LogicalDevice::device, item.supportPresentation,
                                                   item.properties.queueFlags & vk::QueueFlagBits::eGraphics
                                                   ? vk::QueueFlagBits::eGraphics : vk::QueueFlagBits::eCompute,
                                                   item.index));
        }
    } catch (vk::SystemError &error) {
        std::cerr << error.what() << std::endl;
    }


}

const vk::Device &LogicalDevice::getDevice() const {
    return device;
}

std::shared_ptr<PhysicalDevice> LogicalDevice::getBaseDevice() const {
    return baseDevice;
}

std::shared_ptr<LogicalQueue> LogicalDevice::getQueueByType(vk::QueueFlagBits queueType) {
    for (auto &item: queues) {
        if (item->queueType & queueType) {
            return item;
        }
    }
    throw std::runtime_error("Error: no such queue");
}

std::shared_ptr<LogicalQueue> LogicalDevice::getPresentQueue() {
    for (auto &item: queues) {
        if (item->supportPresentation) {
            return item;
        }
    }
    throw std::runtime_error("Error: no such queue");
}

vk::Format LogicalDevice::findDepthFormat() {
    return findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

unsigned int LogicalDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    if(!memoryPropertiesPopulated){
        baseDevice->getBase().getMemoryProperties(&memProperties);
        memoryPropertiesPopulated = true;
    }
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & 1) == 1)
        {
            if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {

                return i;
            }
        }
        typeFilter >>= 1;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

vk::Format LogicalDevice::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                              vk::FormatFeatureFlags features) {
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

unsigned int LogicalDevice::getQueuesAmount() {
    return queues.size();
}

void LogicalDevice::sanitizeQueueCreateInfos(DeviceSuitabilityResults *results) {
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

void LogicalDevice::destroy() {
    for (auto &item: queues){
        item->destroy();
    }
    device.destroy();
    destroyed = true;
}
