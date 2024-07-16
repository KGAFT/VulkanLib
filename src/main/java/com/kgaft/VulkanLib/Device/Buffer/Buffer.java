package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;

public class Buffer {

    public Buffer(LogicalDevice device, LwjglObject<VkBufferCreateInfo> createInfo,
                   LwjglObject<VkMemoryPropertyFlags memoryFlags)
       {
        initialize(createInfo, memoryFlags);
    }

    public Buffer(LogicalDevice device, long size, int usageFlags,
                  int memoryFlags)  {
        initialize(size, usageFlags, memoryFlags);
    }

    public Buffer(const std::shared_ptr<LogicalDevice> &device) : device(device) {
    }
}
