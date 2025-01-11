package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;

import static org.lwjgl.vulkan.VK13.*;

public class Buffer extends DestroyableObject {
    protected static SeriesObject<VkMemoryRequirements> requirements;
    protected static SeriesObject<VkMemoryAllocateInfo> allocInfos;
    protected static SeriesObject<VkMemoryAllocateFlagsInfo> flagsInfos;
    protected static SeriesObject<VkBufferCreateInfo> bufferCreateInfos;

    static {
        try {
            requirements = new SeriesObject<>(VkMemoryRequirements.class);
            allocInfos = new SeriesObject<>(VkMemoryAllocateInfo.class);
            flagsInfos = new SeriesObject<>(VkMemoryAllocateFlagsInfo.class);
            bufferCreateInfos = new SeriesObject<>(VkBufferCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }

    private long buffer = 0;
    private long bufferMemory = 0;
    private LogicalDevice device;

    private long bufferSize;
    private LwjglObject<VkBufferDeviceAddressInfo> addressInfo = new LwjglObject<>(VkBufferDeviceAddressInfo.class);
    private LwjglObject<VkBufferCopy.Buffer> copyRegion = new LwjglObject<>(VkBufferCopy.class, VkBufferCopy.Buffer.class, 1);
    public Buffer(LogicalDevice device, VkBufferCreateInfo createInfo,
                   int memoryFlags) throws IllegalClassFormatException, VkErrorException {
        this.device = device;
        initialize(createInfo, memoryFlags);
    }

    public Buffer(LogicalDevice device, long size, int usageFlags,
                  int memoryFlags) throws IllegalClassFormatException, VkErrorException {
        this.device = device;
        initialize(size, usageFlags, memoryFlags);
    }

    public Buffer(LogicalDevice device) throws IllegalClassFormatException {
        this.device = device;
    }

    public void initialize(long size, int usageFlags, int memoryFlags) throws VkErrorException {
        if (buffer!=0) {
            destroy();
            destroyed = false;
        }
        VkBufferCreateInfo createInfo = bufferCreateInfos.acquireObject();

        createInfo.size(size);
        createInfo.usage(usageFlags);
        createInfo.sharingMode(VK_SHARING_MODE_EXCLUSIVE);

        initialize(createInfo, memoryFlags);

        bufferCreateInfos.releaseObjectInstance(createInfo);
    }

    public long getBufferSize() {
        return bufferSize;
    }

    public void initialize(LwjglObject<VkBufferCreateInfo> createInfo,
                           int memoryFlags) throws VkErrorException {
        initialize(createInfo.get(), memoryFlags);
    }
    private void initialize(VkBufferCreateInfo createInfo,
                            int memoryFlags) throws VkErrorException {
        long[] buffRes = new long[1];
        VkErrorException.checkVkStatus("Failed to create buffer ", vkCreateBuffer(device.getDevice(), createInfo, null, buffRes));
        this.buffer = buffRes[0];
        VkMemoryRequirements memReqs = requirements.acquireObject();
        vkGetBufferMemoryRequirements(device.getDevice(), buffer, memReqs);
        VkMemoryAllocateInfo allocateInfo = allocInfos.acquireObject();
        allocateInfo.allocationSize(bufferSize);
        allocateInfo.memoryTypeIndex(device.findMemoryType(memReqs.memoryTypeBits(), memoryFlags));
        VkMemoryAllocateFlagsInfo flags = flagsInfos.acquireObject();
        flags.flags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
        allocateInfo.pNext(flags);
        VkErrorException.checkVkStatus("Failed to allocate memory ", vkAllocateMemory(device.getDevice(), allocateInfo, null, buffRes));
        this.bufferMemory = buffRes[0];
        vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);
        this.bufferSize = createInfo.size();
        requirements.releaseObjectInstance(memReqs);
        allocInfos.releaseObjectInstance(allocateInfo);
        flagsInfos.releaseObjectInstance(flags);
    }

    public void copyFromBuffer(VkCommandBuffer cmd, Buffer source, long size, long srcOffset, long dstOffset) {
        copyRegion.get().size(size);
        copyRegion.get().srcOffset(srcOffset);
        copyRegion.get().dstOffset(dstOffset);
        vkCmdCopyBuffer(cmd, source.buffer, buffer, copyRegion.get());
        copyRegion.get().get(0).clear();
    }

    public void copyToBuffer(VkCommandBuffer cmd, Buffer destination, long size, long srcOffset, long dstOffset) {
        copyRegion.get().size(size);
        copyRegion.get().srcOffset(srcOffset);
        copyRegion.get().dstOffset(dstOffset);
        vkCmdCopyBuffer(cmd, destination.buffer, buffer, copyRegion.get());
        copyRegion.get().get(0).clear();
    }

    public void map(PointerBuffer output, long offset, int mapFlags) throws VkErrorException {
        VkErrorException.checkVkStatus("Failed to map memory: ", vkMapMemory(device.getDevice(), bufferMemory, offset, bufferSize, mapFlags, output));
    }

    public void unMap(){
        vkUnmapMemory(device.getDevice(), bufferMemory);
    }

    public long getAddress(){
        addressInfo.get().buffer(buffer);
        return vkGetBufferDeviceAddress(device.getDevice(), addressInfo.get());
    }

    public long getBuffer() {
        return buffer;
    }

    @Override
    public void destroy() {
        destroyed = true;
        vkDestroyBuffer(device.getDevice(), buffer, null);
        vkFreeMemory(device.getDevice(), bufferMemory, null);
        try {
            addressInfo = new LwjglObject<>(VkBufferDeviceAddressInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }
}
