package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkBufferCreateInfo;

import java.lang.instrument.IllegalClassFormatException;

import static org.lwjgl.vulkan.VK13.*;

public class UniformBuffer extends Buffer{
    public UniformBuffer(LogicalDevice device, long bufferSize) throws IllegalClassFormatException, VkErrorException {
        VkBufferCreateInfo createInfo = bufferCreateInfos.acquireObject();
        createInfo.sType$Default();
        createInfo.usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        createInfo.size(bufferSize);
        createInfo.sharingMode(VK_SHARING_MODE_EXCLUSIVE);
        super(device, createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        bufferCreateInfos.releaseObjectInstance(createInfo);
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
