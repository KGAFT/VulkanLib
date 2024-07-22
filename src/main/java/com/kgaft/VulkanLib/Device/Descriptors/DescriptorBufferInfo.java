package com.kgaft.VulkanLib.Device.Descriptors;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkDescriptorBufferInfo;

import java.lang.instrument.IllegalClassFormatException;

public class DescriptorBufferInfo {
    private LwjglObject<VkDescriptorBufferInfo.Buffer> base;
    private int binding;
    private int descriptorType;

    public DescriptorBufferInfo(int binding, int descriptorType, int descriptorCount) throws IllegalClassFormatException {
        this.binding = binding;
        this.descriptorType = descriptorType;
        this.base = new LwjglObject<>(VkDescriptorBufferInfo.class, VkDescriptorBufferInfo.Buffer.class, descriptorCount);
    }

    public DescriptorBufferInfo() {
    }

    public LwjglObject<VkDescriptorBufferInfo.Buffer> getBase() {
        return base;
    }

    public void setBase(LwjglObject<VkDescriptorBufferInfo.Buffer> base) {
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
