package com.kgaft.VulkanLib.Device.Descriptors;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkDescriptorBufferInfo;
import org.lwjgl.vulkan.VkDescriptorImageInfo;

import java.lang.instrument.IllegalClassFormatException;

public class DescriptorImageInfo {
    private LwjglObject<VkDescriptorImageInfo.Buffer> base;
    private int binding;
    private int descriptorType;

    public DescriptorImageInfo(int binding, int descriptorType, int descriptorCount) throws IllegalClassFormatException {
        this.binding = binding;
        this.descriptorType = descriptorType;
        this.base = new LwjglObject<>(VkDescriptorImageInfo.class, VkDescriptorImageInfo.Buffer.class, descriptorCount);
    }

    public DescriptorImageInfo() {
    }

    public LwjglObject<VkDescriptorImageInfo.Buffer> getBase() {
        return base;
    }

    public void setBase(LwjglObject<VkDescriptorImageInfo.Buffer> base) {
        this.base = base;
    }

    public int getBinding() {
        return binding;
    }

    public void setBinding(int binding) {
        this.binding = binding;
    }

    public int getDescriptorType() {
        return descriptorType;
    }

    public void setDescriptorType(int descriptorType) {
        this.descriptorType = descriptorType;
    }
}
