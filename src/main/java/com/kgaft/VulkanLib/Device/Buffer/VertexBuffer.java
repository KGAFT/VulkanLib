package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkBufferCreateInfo;
import org.lwjgl.vulkan.VkCommandBuffer;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.ByteBuffer;

import static com.kgaft.VulkanLib.Utils.MemoryUtils.memcpy;
import static org.lwjgl.vulkan.KHRAccelerationStructure.VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
import static org.lwjgl.vulkan.VK13.*;

public class VertexBuffer extends Buffer{
    public VertexBuffer(LogicalDevice device, ByteBuffer vertices, int verticesAmount, long stepSize, int format) throws IllegalClassFormatException, VkErrorException {
        this.vertexCount= verticesAmount;
        this.verticesAmount = verticesAmount;
        this.format = format;
        this.stepSize = stepSize;

                VkBufferCreateInfo createInfo = bufferCreateInfos.acquireObject();
        createInfo.sType$Default();
        createInfo.size(verticesAmount*stepSize);
        createInfo.usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        createInfo.sharingMode(VK_SHARING_MODE_EXCLUSIVE);

        PointerBuffer mapPoint  = PointerBuffer.allocateDirect(1);
        Buffer stagingBuffer = new Buffer(device, createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.map(mapPoint, 0, 0);
        ByteBuffer byteBuffer = mapPoint.getByteBuffer((int) (verticesAmount*stepSize));
        memcpy(byteBuffer, vertices, verticesAmount*stepSize);
        stagingBuffer.unMap();

        createInfo.usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                | (device.isRtEnabled() ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
                | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT : 0));
        super(device, createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer cmd =  device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).beginCommandBuffer();
        copyFromBuffer(cmd, stagingBuffer, createInfo.size(), 0, 0);
        device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).endSingleTimeCommands(cmd);

        stagingBuffer.destroy();
        bufferCreateInfos.releaseObjectInstance(createInfo);


    }
    private int vertexCount;
    private int format;
    private long stepSize;
    private int verticesAmount;

    public void bind(VkCommandBuffer cmd){
        long[] buffers = new long[]{getBuffer()};
        long[] offsets = new long[]{0};
        vkCmdBindVertexBuffers(cmd, 0, buffers, offsets);
    }
    public void drawAll(VkCommandBuffer cmd){
        vkCmdDraw(cmd, vertexCount, 1, 0,0 );
    }

    public int getVertexCount() {
        return vertexCount;
    }

    public int getFormat() {
        return format;
    }

    public long getStepSize() {
        return stepSize;
    }

    public int getVerticesAmount() {
        return verticesAmount;
    }
}
