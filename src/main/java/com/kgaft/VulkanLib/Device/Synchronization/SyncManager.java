package com.kgaft.VulkanLib.Device.Synchronization;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalQueue;
import com.kgaft.VulkanLib.Device.SwapChain;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkCommandBuffer;
import org.lwjgl.vulkan.VkCommandBufferAllocateInfo;
import org.lwjgl.vulkan.VkCommandBufferBeginInfo;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import static org.lwjgl.vulkan.VK13.*;

public class SyncManager extends DestroyableObject {


    private List<VkCommandBuffer> commandBuffers = new ArrayList<>();
    private ThreeFrameSynchronization sync;
    private LogicalDevice device;
    private SwapChain swapChain;
    private LogicalQueue queue;
    private int currentCmd;
    private VkCommandBufferBeginInfo beginInfo = VkCommandBufferBeginInfo.calloc();
    private boolean stop = false;
    private boolean resized = false;
    private int width;
    private int height;
    private IntBuffer curCmd = IntBuffer.allocate(1);
    public SyncManager(LogicalDevice device, SwapChain swapChain, LogicalQueue queue, int maxFramesInFlight) throws IllegalClassFormatException, VkErrorException {
        this.device = device;
        this.swapChain = swapChain;
        this.queue = queue;
        this.sync = new ThreeFrameSynchronization(device, queue, maxFramesInFlight);
        createCommandBuffers(maxFramesInFlight);
    }
    public VkCommandBuffer beginRender(AtomicInteger outCurrentCmd) throws IllegalClassFormatException, VkErrorException {
        if(!stop){
            if(resized){
                setStop(true);
                vkDeviceWaitIdle(device.getDevice());
                swapChain.recreate(width, height);
                resized = false;
                setStop(false);
            }
            currentCmd = sync.prepareForNextImage(swapChain);
            beginInfo.sType$Default();
            vkBeginCommandBuffer(commandBuffers.get(currentCmd), beginInfo);
            outCurrentCmd.set(currentCmd);
            return commandBuffers.get(currentCmd);
        }
        return null;
    }

    public void endRender() throws VkErrorException {
        if(!stop){
            vkEndCommandBuffer(commandBuffers.get(currentCmd));
            curCmd.clear();
            curCmd.put(currentCmd);
            curCmd.rewind();
            sync.submitCommandBuffer(commandBuffers.get(currentCmd), swapChain, curCmd);
            this.currentCmd = curCmd.get();

        }

    }
    public void resize(int width, int height){
        this.width = width;
        this.height = height;
        resized = true;
    }

    public boolean isStop() {
        return stop;
    }

    public void setStop(boolean stop) {
        this.stop = stop;
    }

    private void createCommandBuffers(int maxFramesInFlight) throws IllegalClassFormatException, VkErrorException {
        LwjglObject<VkCommandBufferAllocateInfo> allocInfo = new LwjglObject<>(VkCommandBufferAllocateInfo.class);
        allocInfo.get().level(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        allocInfo.get().commandPool(queue.getCommandPool());
        allocInfo.get().commandBufferCount(maxFramesInFlight);
        PointerBuffer pb = PointerBuffer.allocateDirect(maxFramesInFlight);
        VkErrorException.checkVkStatus("Failed to allocate command buffers", vkAllocateCommandBuffers(device.getDevice(), allocInfo.get(), pb));
        while(pb.hasRemaining()){
            commandBuffers.add(new VkCommandBuffer(pb.get(), device.getDevice()));
        }

    }

    @Override
    public void destroy() {
        destroyed = true;
        sync.destroy();
    }
}
