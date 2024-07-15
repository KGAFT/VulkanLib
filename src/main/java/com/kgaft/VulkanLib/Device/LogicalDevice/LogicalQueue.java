package com.kgaft.VulkanLib.Device.LogicalDevice;

import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.*;
import static org.lwjgl.vulkan.VK13.*;
import java.lang.instrument.IllegalClassFormatException;

public class LogicalQueue extends DestroyableObject {
    private VkQueue queue;
    private boolean supportPresentation;
    private int queueType;
    private long commandPool;
    private VkDevice device;
    private int index;
    private LwjglObject<VkCommandBufferAllocateInfo> allocInfo;
    private LwjglObject<VkCommandBufferBeginInfo> beginInfo;
    private LwjglObject<VkSubmitInfo> submitInfo;
    public LogicalQueue(VkQueue queue, VkDevice device, boolean supportPresentation, int queueType, int index) throws VkErrorException {
        try
        {
            this.device = device;
            this.queue = queue;
            this.supportPresentation = supportPresentation;
            this.queueType = queueType;
            allocInfo = new LwjglObject<>(VkCommandBufferAllocateInfo.class);
            beginInfo = new LwjglObject<>(VkCommandBufferBeginInfo.class);
            submitInfo = new LwjglObject<>(VkSubmitInfo.class);

            LwjglObject<VkCommandPoolCreateInfo> poolInfo = new LwjglObject<>(VkCommandPoolCreateInfo.class);
            poolInfo.get().queueFamilyIndex(index);
            poolInfo.get().flags(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT|VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            long[] handleRes = new long[1];
            int status = vkCreateCommandPool(device, poolInfo.get(), null, handleRes);
            if(status!=VK_SUCCESS){
                throw new VkErrorException("Failed to create command pool for queue: "+index+" ", status);
            }
            commandPool = handleRes[0];
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }

    public VkCommandBuffer beginCommandBuffer(){
        allocInfo.get().level(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        allocInfo.get().commandPool(commandPool);
        allocInfo.get().commandBufferCount(1);
        PointerBuffer pb = PointerBuffer.allocateDirect(1);
        vkAllocateCommandBuffers(device, allocInfo.get(), pb);
        beginInfo.get().flags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkCommandBuffer cmd = new VkCommandBuffer(pb.get(), device);
        pb.free();
        vkBeginCommandBuffer(cmd, beginInfo.get());
        return cmd;
    }


    public void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);
        PointerBuffer pb = PointerBuffer.allocateDirect(1);
        pb.put(commandBuffer);
        submitInfo.get().pCommandBuffers(pb);
        vkQueueSubmit(queue, submitInfo.get(), 0);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device, commandPool, commandBuffer);
        pb.free();
    }

    public VkQueue getQueue() {
        return queue;
    }

    public boolean isSupportPresentation() {
        return supportPresentation;
    }

    public int getQueueType() {
        return queueType;
    }

    public VkDevice getDevice() {
        return device;
    }

    public int getIndex() {
        return index;
    }

    @Override
    public void destroy() {
        destroyed = true;
        vkDestroyCommandPool(device, commandPool, null);
    }
}
