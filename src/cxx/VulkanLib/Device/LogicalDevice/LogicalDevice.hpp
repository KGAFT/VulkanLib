//
// Created by kgaft on 11/5/23.
//
#ifndef VULKANLIB_LOGICALDEVICE_HPP
#define VULKANLIB_LOGICALDEVICE_HPP

#include <memory>

#include "VulkanLib/Device/PhysicalDevice/DeviceSuitability.hpp"
#include "VulkanLib/MemoryUtils/MemoryUtils.hpp"
#include "LogicalQueue.hpp"

class LogicalDevice : public IDestroyableObject {
private:
    static inline auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();
    static inline float priority = 1.0f;
    static inline unsigned int usedQueueCreateInfos;
public:
    LogicalDevice(Instance &instance, std::shared_ptr<PhysicalDevice> device, DeviceBuilder &builder, DeviceSuitabilityResults *results);

    LogicalDevice() = default;

private:
    vk::Device device;
    std::vector<std::shared_ptr<LogicalQueue>> queues;
    std::shared_ptr<PhysicalDevice> baseDevice;
    vk::PhysicalDeviceMemoryProperties memProperties;
    bool memoryPropertiesPopulated = false;
public:
    const vk::Device &getDevice() const;

    std::shared_ptr<PhysicalDevice> getBaseDevice() const;

    std::shared_ptr<LogicalQueue> getQueueByType(vk::QueueFlagBits queueType);

    std::shared_ptr<LogicalQueue> getPresentQueue();

    vk::Format findDepthFormat();

    unsigned int findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    vk::Format findSupportedFormat(
            const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    unsigned int getQueuesAmount();

private:
    void sanitizeQueueCreateInfos(DeviceSuitabilityResults *results);
public:
    void destroy() override;


};

#endif
