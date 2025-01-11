package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkBufferCreateInfo;

import java.lang.instrument.IllegalClassFormatException;

import static org.lwjgl.vulkan.VK13.*;

public class IndirectBuffer extends Buffer{
    public IndirectBuffer(LogicalDevice device, int structuresAmount, int sizeOfStructure) throws IllegalClassFormatException, VkErrorException {
        this.structuresAmount = structuresAmount;
        this.sizeOfStructure = sizeOfStructure;

        VkBufferCreateInfo createInfo = bufferCreateInfos.acquireObject();
        createInfo.sType$Default();
        createInfo.size((long) structuresAmount *sizeOfStructure);
        createInfo.usage(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        createInfo.sharingMode(VK_SHARING_MODE_EXCLUSIVE);
        super(device, createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        bufferCreateInfos.releaseObjectInstance(createInfo);
        map(mapPoint, 0, 0);
    }
    private PointerBuffer mapPoint = PointerBuffer.allocateDirect(1);
    private int structuresAmount;
    private int sizeOfStructure;

    public PointerBuffer getMapPoint() {
        return mapPoint;
    }

    public int getStructuresAmount() {
        return structuresAmount;
    }

    public int getSizeOfStructure() {
        return sizeOfStructure;
    }

    @Override
    public void destroy() {
        unMap();
        super.destroy();
    }
}
