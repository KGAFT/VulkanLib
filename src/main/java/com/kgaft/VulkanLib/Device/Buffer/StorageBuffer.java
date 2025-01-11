package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;

import java.lang.instrument.IllegalClassFormatException;

import static org.lwjgl.vulkan.VK13.*;

public class StorageBuffer extends Buffer {
    public StorageBuffer(LogicalDevice device, long bufferSize, int additionalUsageFlags) throws IllegalClassFormatException, VkErrorException {
        super(device,
                bufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                        additionalUsageFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        map(mapPoint, 0, 0);

    }
    private PointerBuffer mapPoint = PointerBuffer.allocateDirect(1);


    public PointerBuffer getMapPoint() {
        return mapPoint;
    }

    @Override
    public void destroy() {
        unMap();
        super.destroy();
    }
}
