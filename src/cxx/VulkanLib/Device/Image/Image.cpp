//
// Created by kgaft on 3/11/24.
//
#include "Image.hpp"

Image::Image(std::shared_ptr<LogicalDevice> device, vk::Image base) : device(std::move(device)), base(base) {
    castCreated = true;
}

Image::Image(std::shared_ptr<LogicalDevice> &device, vk::ImageCreateInfo &createInfo) : imageInfo(createInfo) {
    initialize(device, createInfo);
}

Image::Image() {

}

std::shared_ptr<ImageView> Image::createImageView(vk::ImageViewCreateInfo &createInfo) {
    vk::ImageView view = Image::device->getDevice().createImageView(createInfo, VkLibAlloc::acquireAllocCb().get());
    imageViews.push_back(std::make_shared<ImageView>(imageInfo,
                                                     Image::device, view, createInfo));
    return imageViews[imageViews.size() - 1];
}

void Image::initialize(std::shared_ptr<LogicalDevice> device, vk::Image base) {
    Image::device = device;
    Image::base = base;
}

void Image::initialize(std::shared_ptr<LogicalDevice> &device, vk::ImageCreateInfo &createInfo) {
    vk::Result res;
    Image::device = device;
    Image::imageInfo = createInfo;
    base =  Image::device->getDevice().createImage(createInfo, VkLibAlloc::acquireAllocCb().get());

    vk::MemoryRequirements requirements;
    Image::device->getDevice().getImageMemoryRequirements(base, &requirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex =  Image::device->findMemoryType(requirements.memoryTypeBits,
                                                               vk::MemoryPropertyFlagBits::eDeviceLocal);

    res =  Image::device->getDevice().allocateMemory(&allocInfo, VkLibAlloc::acquireAllocCb().get(), &imageMemory);
    if(res!=vk::Result::eSuccess){
        throw std::runtime_error("Failed to allocate image memory");
    }
    Image::device->getDevice().bindImageMemory(base, imageMemory, 0);

}

vk::Image &Image::getBase() {
    return base;
}

void Image::copyFromBuffer(Buffer &buffer, uint32_t layerCount, LogicalQueue &queue) {
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

void Image::transitionImageLayout(std::shared_ptr<LogicalDevice> device, vk::ImageLayout oldLayout,
                                  vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags) {
    vk::CommandBuffer commandBuffer = device->getQueueByType(vk::QueueFlagBits::eGraphics)->beginSingleTimeCommands();

    transitionImageLayout(commandBuffer, oldLayout, newLayout, aspectFlags);

    device->getQueueByType(vk::QueueFlagBits::eGraphics)->endSingleTimeCommands(commandBuffer);
}

void Image::transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                  vk::ImageAspectFlags aspectFlags) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = static_cast<VkImageLayout>(oldLayout);
    barrier.newLayout = static_cast<VkImageLayout>(newLayout);
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = base;
    barrier.subresourceRange.aspectMask = (unsigned int)aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (static_cast<VkImageLayout>(oldLayout) == VK_IMAGE_LAYOUT_UNDEFINED && static_cast<VkImageLayout>(newLayout) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (static_cast<VkImageLayout>(oldLayout) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             static_cast<VkImageLayout>(newLayout) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
}

std::vector<std::shared_ptr<ImageView>> &Image::getImageViews() { return imageViews; }

void Image::resize(uint32_t width, uint32_t height) {
    if (!castCreated) {
        destroy();
        destroyed = false;
        imageInfo.extent = vk::Extent3D{width, height, 1};
        if(imageInfo.initialLayout!=vk::ImageLayout::eUndefined){
            vk::ImageLayout tmpLayout = imageInfo.initialLayout;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            initialize(device, imageInfo);
            transitionImageLayout(device, vk::ImageLayout::eUndefined, tmpLayout, vk::ImageAspectFlagBits::eColor);
            imageInfo.initialLayout = tmpLayout;
        } else {
            initialize(device, imageInfo);
        }
        for (auto &item: imageViews) {
            item->createInfo.image = base;
            item->base = device->getDevice().createImageView(item->createInfo, VkLibAlloc::acquireAllocCb().get());
            item->parentInfo = imageInfo;
        }
    }

}

vk::ImageCreateInfo &Image::getImageInfo() {
    if (castCreated) {
        throw std::runtime_error("Error: you cannot get image info, from image created via cast");
    }
    return imageInfo;
}

void Image::destroy() {
    if (!castCreated) {
        for (auto &item: imageViews) {
            item->destroy();
        }
        device->getDevice().destroyImage(base, VkLibAlloc::acquireAllocCb().get());
        device->getDevice().freeMemory(imageMemory, VkLibAlloc::acquireAllocCb().get());
    }
    destroyed = true;
}
