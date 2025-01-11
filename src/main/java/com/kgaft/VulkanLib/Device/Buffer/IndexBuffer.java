package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.MemoryUtils;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkBufferCreateInfo;
import org.lwjgl.vulkan.VkCommandBuffer;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.ByteBuffer;

import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRAccelerationStructure.*;

public class IndexBuffer extends Buffer {
    public IndexBuffer(LogicalDevice device, ByteBuffer indices, int indexCount, int indexType) throws IllegalClassFormatException, VkErrorException {
        this.device = device;
        this.indexCount = indexCount;
        this.indexType = indexType;

        VkBufferCreateInfo createInfo = bufferCreateInfos.acquireObject();
        createInfo.sType$Default();
        createInfo.size((long) indexCount * Integer.BYTES);
        createInfo.usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        createInfo.sharingMode(VK_SHARING_MODE_EXCLUSIVE);


        Buffer stagingBuffer = new Buffer(device, createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        PointerBuffer pb = PointerBuffer.allocateDirect(1);
        stagingBuffer.map(pb, 0, 0);
        MemoryUtils.memcpy(pb.getByteBuffer((int) stagingBuffer.getBufferSize()), indices, stagingBuffer.getBufferSize());
        stagingBuffer.unMap();
        createInfo.usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                | (device.isRtEnabled() ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
                | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT : 0));
        super(device, createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkCommandBuffer cmd = device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).beginCommandBuffer();
        copyFromBuffer(cmd, stagingBuffer, createInfo.size(), 0, 0);
        device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).endSingleTimeCommands(cmd);
        bufferCreateInfos.releaseObjectInstance(createInfo);
        stagingBuffer.destroy();
    }

    private LogicalDevice device;
    private int indexCount;
    private int indexType;

    public void bind(VkCommandBuffer cmd){
        vkCmdBindIndexBuffer(cmd, getBuffer(), 0, indexType);
    }

    public void drawAll(VkCommandBuffer cmd){
        vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
    }

    public LogicalDevice getDevice() {
        return device;
    }

    public int getIndexCount() {
        return indexCount;
    }

    public int getIndexType() {
        return indexType;
    }
}
