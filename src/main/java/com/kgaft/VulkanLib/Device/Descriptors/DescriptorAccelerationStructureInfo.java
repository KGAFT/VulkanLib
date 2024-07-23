package com.kgaft.VulkanLib.Device.Descriptors;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkDescriptorBufferInfo;
import org.lwjgl.vulkan.VkWriteDescriptorSetAccelerationStructureKHR;

import java.lang.instrument.IllegalClassFormatException;

public class DescriptorAccelerationStructureInfo {
    private LwjglObject<VkWriteDescriptorSetAccelerationStructureKHR> base;
    private int binding;
    private int descriptorType;

    public DescriptorAccelerationStructureInfo(int binding, int descriptorType) throws IllegalClassFormatException {
        this.binding = binding;
        this.descriptorType = descriptorType;
        this.base = new LwjglObject<>(VkWriteDescriptorSetAccelerationStructureKHR.class);
    }

    public DescriptorAccelerationStructureInfo() {
    }

    public LwjglObject<VkWriteDescriptorSetAccelerationStructureKHR> getBase() {
        return base;
    }

    public void setBase(LwjglObject<VkWriteDescriptorSetAccelerationStructureKHR> base) {
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
