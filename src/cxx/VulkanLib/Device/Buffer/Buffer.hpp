//
// Created by kgaft on 11/19/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanLib/MemoryUtils/IDestroyableObject.hpp"
#include "VulkanLib/Device/LogicalDevice/LogicalDevice.hpp"

class Buffer : IDestroyableObject {
public:
    Buffer(LogicalDevice& device, vk::BufferCreateInfo& createInfo){

    }
private:
    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;
};


