package com.kgaft.VulkanLib.Device.Descriptors;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.VkDescriptorPoolCreateInfo;
import org.lwjgl.vulkan.VkDescriptorPoolSize;
import org.lwjgl.vulkan.VkDescriptorSetAllocateInfo;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.LongBuffer;
import java.util.ArrayList;
import java.util.List;
import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRAccelerationStructure.VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

public class DescriptorPool extends DestroyableObject {
    private long descriptorPool;
    private LogicalDevice logicalDevice;
    private SeriesObject<VkDescriptorSetAllocateInfo> allocInfos;
    private boolean supportAccelerationStructures;
    private LongBuffer layoutsBuffer = null;
    public DescriptorPool(LogicalDevice logicalDevice, boolean supportAccelerationStructure) throws IllegalClassFormatException, VkErrorException {
        this.logicalDevice = logicalDevice;
        this.supportAccelerationStructures = supportAccelerationStructure;
        LwjglObject<VkDescriptorPoolSize.Buffer> poolSizes = new LwjglObject<>(VkDescriptorPoolSize.class, VkDescriptorPoolSize.Buffer.class, 11+(supportAccelerationStructures?1:0));
        for(int i = 0; i<=10; i++){
                poolSizes.get().get(i).descriptorCount(1000);
                poolSizes.get().get(i).type(i);
        }
        if(supportAccelerationStructure){
            poolSizes.get().get(11).descriptorCount(1000);
            poolSizes.get().get(11).type(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
        }
        poolSizes.get().rewind();

        LwjglObject<VkDescriptorPoolCreateInfo> createInfo = new LwjglObject<>(VkDescriptorPoolCreateInfo.class);
        createInfo.get().maxSets(1500);
        createInfo.get().pPoolSizes(poolSizes.get());
        long[] buffer = new long[1];
        VkErrorException.checkVkStatus("Failed to create descriptor pool ", vkCreateDescriptorPool(logicalDevice.getDevice(), createInfo.get(), null, buffer));
        this.descriptorPool = buffer[0];
    }

    public DescriptorSet allocateDescriptorSet(int instanceCount, long layout) throws VkErrorException {
        DescriptorSet res = new DescriptorSet();
        layoutsBuffer = LongBuffer.allocate(instanceCount);
        for (int i = 0; i < instanceCount; i++) {
            layoutsBuffer.put(layout);
        }
        layoutsBuffer.rewind();
        VkDescriptorSetAllocateInfo allocateInfo = allocInfos.acquireObject();

        allocateInfo.descriptorPool(descriptorPool);
        allocateInfo.pSetLayouts(layoutsBuffer);

        res.descriptorSets = new long[instanceCount];
        res.device = logicalDevice;
        VkErrorException.checkVkStatus("Failed to allocate descriptors ", vkAllocateDescriptorSets(logicalDevice.getDevice(), allocateInfo, res.descriptorSets));
        return res;
    }

}
