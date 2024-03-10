//
// Created by kgaft on 11/28/23.
//
#ifndef VULKANLIB_DESCRIPTORPOOL_HPP
#define VULKANLIB_DESCRIPTORPOOL_HPP

#include <vulkan/vulkan.hpp>
#include <memory>
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"
#include "VulkanLib/MemoryUtils/SeriesObject.hpp"
#include "DescriptorSet.hpp"

class DescriptorPool : public IDestroyableObject {
private:
    static inline auto instance = std::shared_ptr<DescriptorPool>();
    static inline bool initialized = false;
public:
    static std::shared_ptr<DescriptorPool> getInstance(std::shared_ptr<LogicalDevice> device);
private:
    DescriptorPool( std::shared_ptr<LogicalDevice> logicalDevice);
    DescriptorPool() = default;
private:
    vk::DescriptorPool descriptorPool;
    std::shared_ptr<LogicalDevice> logicalDevice;
    std::vector<vk::DescriptorSetLayout> layouts;
    SeriesObject<vk::DescriptorSetAllocateInfo> allocInfos;
public:
    std::shared_ptr<DescriptorSet> allocateDescriptorSet(uint32_t instanceCount, vk::DescriptorSetLayout layout);

public:
    void destroy() override;
};

#endif

