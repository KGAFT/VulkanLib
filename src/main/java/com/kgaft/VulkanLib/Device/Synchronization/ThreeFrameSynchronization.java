package com.kgaft.VulkanLib.Device.Synchronization;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalQueue;
import com.kgaft.VulkanLib.Device.SwapChain;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRSwapchain.*;
public class ThreeFrameSynchronization extends DestroyableObject {

    
    private int maxFramesInFlight;
    private List<Long> imageAvailableSemaphores;
    private List<Long> renderFinishedSemaphores;
    private List<Long> inFlightFences;
    private List<Long> imagesInFlight;
    private LogicalDevice device;
    private LogicalQueue presentQueue;
    private int currentFrame = 0;
    private SeriesObject<VkSubmitInfo> submitInfos = new SeriesObject<>(VkSubmitInfo.class);
    private SeriesObject<VkPresentInfoKHR> presentInfos = new SeriesObject<>(VkPresentInfoKHR.class);
    private LongBuffer waitSemaphore = LongBuffer.allocate(1);
    private LongBuffer signalSemaphore = LongBuffer.allocate(1);
    private LongBuffer swapchainBuff = LongBuffer.allocate(1);
    private IntBuffer waitStages = IntBuffer.allocate(1);
    private PointerBuffer cmdBuff = PointerBuffer.allocateDirect(1);
    public ThreeFrameSynchronization(LogicalDevice device, LogicalQueue graphicsQueue,
                                     int maxFramesInFlight) throws IllegalClassFormatException {
        this.device = device;
        this.presentQueue = graphicsQueue;
        this.maxFramesInFlight = maxFramesInFlight;
        createSyncObjects();
    }

    public int prepareForNextImage(SwapChain swapChain) throws VkErrorException {
        int res = 0;
        res = vkWaitForFences(device.getDevice(), inFlightFences.get(currentFrame), true, Long.MAX_VALUE);
        VkErrorException.checkVkStatus("Failed to wait for fences", res);
        int[] buff = new int[1];
        res = vkAcquireNextImageKHR(device.getDevice(), swapChain.getSwapchainKhr(), Long.MAX_VALUE, imageAvailableSemaphores.get(currentFrame), 0, buff);
        VkErrorException.checkVkStatus("Failed to acquire next image", res);
        return buff[0];
    }

    public void submitCommandBuffer(VkCommandBuffer cmd, SwapChain swapChain, IntBuffer currentImage) throws VkErrorException {
        int res;
        if (imagesInFlight.get(currentImage.get(0)) != VK_NULL_HANDLE) {
            res = vkWaitForFences(device.getDevice(), imagesInFlight.get(currentImage.get(0)), true, Long.MAX_VALUE);

            VkErrorException.checkVkStatus("Failed to wait submit fence", res);
        }
        imagesInFlight.set(currentImage.get(0), inFlightFences.get(currentFrame));

        VkSubmitInfo submitInfo = submitInfos.acquireObject();
        
        waitStages.clear();
        waitStages.put(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        waitStages.rewind();
        waitSemaphore.clear();
        waitSemaphore.put(imageAvailableSemaphores.get(currentFrame));
        waitSemaphore.rewind();
        cmdBuff.clear();
        cmdBuff.put(cmd.address());
        cmdBuff.rewind();
        signalSemaphore.clear();
        signalSemaphore.put(renderFinishedSemaphores.get(currentFrame));
        signalSemaphore.rewind();
        submitInfo.pWaitSemaphores(waitSemaphore);
        submitInfo.pWaitDstStageMask(waitStages);

        submitInfo.pCommandBuffers(cmdBuff);
        submitInfo.pSignalSemaphores(signalSemaphore);
        
        res = vkResetFences(device.getDevice(), inFlightFences.get(currentFrame));
        VkErrorException.checkVkStatus("Failed to reset fence", res);
        res = vkQueueSubmit(presentQueue.getQueue(), submitInfo, inFlightFences.get(currentFrame));
        VkErrorException.checkVkStatus("Failed to submit queue ", res);
        VkPresentInfoKHR presentInfo = presentInfos.acquireObject();
        swapchainBuff.clear();
        swapchainBuff.put(swapChain.getSwapchainKhr());
        swapchainBuff.rewind();
        signalSemaphore.rewind();
        presentInfo.pWaitSemaphores(signalSemaphore);
        presentInfo.pSwapchains(swapchainBuff);

        presentInfo.pImageIndices(currentImage);
        res = vkQueuePresentKHR(presentQueue.getQueue(), presentInfo);
        VkErrorException.checkVkStatus("Failed to present swapchain ", res);
        currentFrame = (currentFrame + 1) % maxFramesInFlight;
        presentInfos.releaseObjectInstance(presentInfo);
        submitInfos.releaseObjectInstance(submitInfo);
    }


    private void createSyncObjects() throws IllegalClassFormatException {
        LwjglObject<VkSemaphoreCreateInfo> semaphoreInfo = new LwjglObject<>(VkSemaphoreCreateInfo.class);

        LwjglObject<VkFenceCreateInfo> fenceInfo = new LwjglObject<>(VkFenceCreateInfo.class);
        fenceInfo.get().flags(VK_FENCE_CREATE_SIGNALED_BIT);
        long[] buff = new long[1];
        for (int i = 0; i < maxFramesInFlight; i++) {
            vkCreateSemaphore(device.getDevice(), semaphoreInfo.get(), null, buff);
            imageAvailableSemaphores.add(buff[0]);
            vkCreateSemaphore(device.getDevice(), semaphoreInfo.get(), null, buff);
            renderFinishedSemaphores.add(buff[0]);
            vkCreateFence(device.getDevice(), fenceInfo.get(), null, buff);
            inFlightFences.add(buff[0]);
            imagesInFlight.add(0L);
        }
    }

    @Override
    public void destroy() {
        vkDeviceWaitIdle(device.getDevice());
        imageAvailableSemaphores.forEach(element->{
            vkDestroySemaphore(device.getDevice(), element, null);
        });
        renderFinishedSemaphores.forEach(element->{
            vkDestroySemaphore(device.getDevice(), element, null);
        });
        inFlightFences.forEach(element->{
            vkDestroyFence(device.getDevice(), element, null);
        });
        imageAvailableSemaphores.clear();
        renderFinishedSemaphores.clear();
        inFlightFences.clear();
        destroyed = true;
    }
}
