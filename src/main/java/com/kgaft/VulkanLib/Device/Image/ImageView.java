package com.kgaft.VulkanLib.Device.Image;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkImageCreateInfo;
import org.lwjgl.vulkan.VkImageViewCreateInfo;

import static org.lwjgl.vulkan.VK13.*;

public class ImageView extends DestroyableObject {
    protected LwjglObject<VkImageCreateInfo> parentInfo;
    protected LogicalDevice device;
    protected long imageView;
    protected LwjglObject<VkImageViewCreateInfo> createInfo;

    public ImageView(LogicalDevice device, long imageView, LwjglObject<VkImageViewCreateInfo> createInfo, LwjglObject<VkImageCreateInfo> parentInfo) {
        this.device = device;
        this.imageView = imageView;
        this.createInfo = createInfo;
        this.parentInfo = parentInfo;
    }

    public long getImageView() {
        return imageView;
    }

    public LwjglObject<VkImageViewCreateInfo> getCreateInfo() {
        return createInfo;
    }

    public LwjglObject<VkImageCreateInfo> getParentInfo() {
        return parentInfo;
    }

    protected void setDestroyed(boolean val){
        this.destroyed = val;
    }
    @Override
    public void destroy() {
        destroyed = true;
        vkDestroyImageView(device.getDevice(), imageView, null);
    }
}
