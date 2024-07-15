package com.kgaft.VulkanLib.Device.Image;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
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

    public Image(LogicalDevice device, long base) {
        castCreated = true;
        this.device = device;
        this.base = base;
    }

    public Image(LogicalDevice device, LwjglObject<VkImageCreateInfo> createInfo) throws VkErrorException {
        initialize(device, createInfo);
    }

    public Image() {
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
    public void copyFromBuffer(Buffer &buffer, uint32_t layerCount, LogicalQueue &queue) {
        vk::CommandBuffer cmd = queue.beginSingleTimeCommands();
        transitionImageLayout(cmd, imageInfo.initialLayout, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;
        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;
        region.imageExtent = imageInfo.extent;
        cmd.copyBufferToImage(buffer.getBuffer(), base, vk::ImageLayout::eTransferDstOptimal, 1, &region);
        transitionImageLayout(cmd,vk::ImageLayout::eTransferDstOptimal,  vk::ImageLayout::eGeneral, vk::ImageAspectFlagBits::eColor);
        imageInfo.initialLayout = vk::ImageLayout::eGeneral;
        queue.endSingleTimeCommands(cmd);
    }
}
