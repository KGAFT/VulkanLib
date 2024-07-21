package com.kgaft.VulkanLib.Device.Image;

import com.kgaft.VulkanLib.Device.Buffer.Buffer;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalQueue;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;

import static org.lwjgl.vulkan.VK10.*;

public class Image extends DestroyableObject {
    private LogicalDevice device;
    private long base;
    private LwjglObject<VkImageCreateInfo> imageInfo;
    private ArrayList<ImageView> imageViews = new ArrayList<>();
    private long imageMemory;
    private boolean castCreated = false;
    private LwjglObject<VkBufferImageCopy.Buffer> region = new LwjglObject<>(VkBufferImageCopy.class, VkBufferImageCopy.Buffer.class, 1);
    public Image(LogicalDevice device, long base) throws IllegalClassFormatException {
        castCreated = true;
        this.device = device;
        this.base = base;
    }

    public Image(LogicalDevice device, LwjglObject<VkImageCreateInfo> createInfo) throws VkErrorException, IllegalClassFormatException {
        initialize(device, createInfo);
    }

    public Image() throws IllegalClassFormatException {
    }

    public void initialize(LogicalDevice device, long image) {
        this.device = device;
        this.base = image;
    }

    public void initialize(LogicalDevice device, LwjglObject<VkImageCreateInfo> createInfo) throws VkErrorException {
        this.imageInfo = createInfo;
        this.device = device;
        long[] res = new long[1];
        int status = vkCreateImage(device.getDevice(), createInfo.get(), null, res);
        if (status != VK_SUCCESS) {
            throw new VkErrorException("Failed to create image", status);
        }
        this.base = res[0];

        try {
            LwjglObject<VkMemoryRequirements> reqs = new LwjglObject<>(VkMemoryRequirements.class);

            vkGetImageMemoryRequirements(device.getDevice(), base, reqs.get());
            LwjglObject<VkMemoryAllocateInfo> allocInfo = new LwjglObject<>(VkMemoryAllocateInfo.class);
            allocInfo.get().allocationSize(reqs.get().size());
            allocInfo.get().memoryTypeIndex(device.findMemoryType(reqs.get().memoryTypeBits(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

            status = vkAllocateMemory(device.getDevice(), allocInfo.get(), null, res);
            if (status != VK_SUCCESS) {
                throw new VkErrorException("Failed to allocate image memory", status);
            }
            this.imageMemory = res[0];

        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }

    public ImageView createImageView(LwjglObject<VkImageViewCreateInfo> createInfo) throws VkErrorException {
        long[] res = new long[1];
        int status = vkCreateImageView(device.getDevice(), createInfo.get(), null, res);
        if (status != VK_SUCCESS) {
            throw new VkErrorException("Failed to create image view: ", status);
        }
        ImageView result = new ImageView(device, res[0], createInfo, this.imageInfo);
        imageViews.add(result);
        return result;
    }


    public void transitionImageLayout(LogicalDevice device, int oldLayout, int newLayout,
                                      int aspectFlags) {
        VkCommandBuffer commandBuffer = device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).beginCommandBuffer();

        transitionImageLayout(commandBuffer, oldLayout, newLayout, aspectFlags);

        device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).endSingleTimeCommands(commandBuffer);
    }

    public void transitionImageLayout(VkCommandBuffer commandBuffer, int oldLayout, int newLayout,
                                      int aspectFlags) {
        try {
            LwjglObject<VkImageMemoryBarrier.Buffer> barrier = new LwjglObject<>(VkImageMemoryBarrier.class, VkImageMemoryBarrier.Buffer.class, 1);
            barrier.get().oldLayout(oldLayout);
            barrier.get().newLayout(newLayout);
            barrier.get().srcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.get().dstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.get().image(base);
            barrier.get().subresourceRange().aspectMask(aspectFlags);
            barrier.get().subresourceRange().baseMipLevel(0);
            barrier.get().subresourceRange().levelCount(1);
            barrier.get().subresourceRange().baseArrayLayer(0);
            barrier.get().subresourceRange().layerCount(1);

            int sourceStage;
            int destinationStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.get().srcAccessMask(0);
                barrier.get().dstAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT);

                sourceStage = (VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                    newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.get().srcAccessMask(VK_ACCESS_TRANSFER_WRITE_BIT);
                barrier.get().dstAccessMask(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                barrier.get().srcAccessMask(0);
                barrier.get().dstAccessMask(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            vkCmdPipelineBarrier(
                    commandBuffer,
                    sourceStage, destinationStage,
                    0,
                    null,
                    null, barrier.get());

        } catch (Exception e) {
            throw new RuntimeException(e);
        }

    }
    public void copyFromBuffer(Buffer buffer, int layerCount, LogicalQueue queue) {
        VkCommandBuffer cmd = queue.beginCommandBuffer();
        transitionImageLayout(cmd, imageInfo.get().initialLayout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

        region.get().bufferOffset(0);
        region.get().bufferRowLength(0);
        region.get().bufferImageHeight(0);
        region.get().imageSubresource().aspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
        region.get().imageSubresource().mipLevel(0);
        region.get().imageSubresource().baseArrayLayer(0);
        region.get().imageSubresource().layerCount(layerCount);
        region.get().imageOffset().x(0);
        region.get().imageOffset().y(0);
        region.get().imageOffset().z(0);
        region.get().imageExtent(imageInfo.get().extent());
        vkCmdCopyBufferToImage(cmd, buffer.getBuffer(), base, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region.get());
        transitionImageLayout(cmd,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
        imageInfo.get().initialLayout(VK_IMAGE_LAYOUT_GENERAL);
        queue.endSingleTimeCommands(cmd);
    }

    public void resize(int width, int height) throws VkErrorException {
        if (!castCreated) {
            destroy();
            destroyed = false;
            imageInfo.get().extent().width(width);
            imageInfo.get().extent().height(height);
            imageInfo.get().extent().depth(1);
            if(imageInfo.get().initialLayout()!=VK_IMAGE_LAYOUT_UNDEFINED){
                int tmpLayout = imageInfo.get().initialLayout();
                imageInfo.get().initialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
                initialize(device, imageInfo);
                transitionImageLayout(device, VK_IMAGE_LAYOUT_UNDEFINED, tmpLayout, VK_IMAGE_ASPECT_COLOR_BIT);
                imageInfo.get().initialLayout(tmpLayout);
            } else {
                initialize(device, imageInfo);
            }
            imageViews.forEach(element->{
                element.createInfo.get().image(base);
                long[] res = new long[1];
                vkCreateImageView(device.getDevice(), element.createInfo.get(), null, res);
                element.imageView = res[0];
                element.parentInfo = imageInfo;
            });

        }

    }

    public long getBase() {
        return base;
    }

    public LwjglObject<VkImageCreateInfo> getImageInfo() {
        return imageInfo;
    }

    public ArrayList<ImageView> getImageViews() {
        return imageViews;
    }

    @Override
    public void destroy() {
        if(!castCreated){
            imageViews.forEach(ImageView::destroy);
            vkDestroyImage(device.getDevice(), base, null);
            vkFreeMemory(device.getDevice(), imageMemory, null);
        }
        destroyed = true;
    }
}
