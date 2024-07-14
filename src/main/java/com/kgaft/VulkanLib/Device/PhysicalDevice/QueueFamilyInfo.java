package com.kgaft.VulkanLib.Device.PhysicalDevice;

import org.lwjgl.vulkan.VkQueueFamilyProperties;

public class QueueFamilyInfo {
    public int index;
    public VkQueueFamilyProperties properties;
    public boolean supportPresentation;

    public QueueFamilyInfo(int index, VkQueueFamilyProperties properties, boolean supportPresentation) {
        this.index = index;
        this.properties = properties;
        this.supportPresentation = supportPresentation;
    }

    public QueueFamilyInfo() {
    }
}
