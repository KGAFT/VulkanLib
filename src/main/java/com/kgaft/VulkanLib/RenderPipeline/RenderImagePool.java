package com.kgaft.VulkanLib.RenderPipeline;

import com.kgaft.VulkanLib.Device.Image.Image;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.VkImageCreateInfo;
import org.lwjgl.vulkan.VkImageViewCreateInfo;

import java.lang.instrument.IllegalClassFormatException;
import java.util.HashMap;
import java.util.Map;

import static org.lwjgl.vulkan.VK13.*;

public class RenderImagePool extends DestroyableObject {
    private static LwjglObject<VkImageCreateInfo> defaultDepthCreateInfo;
    private static LwjglObject<VkImageCreateInfo> defaultColorCreateInfo;
    private static LwjglObject<VkImageViewCreateInfo> defaultColorViewCreateInfo;
    private static LwjglObject<VkImageViewCreateInfo> defaultDepthViewCreateInfo;


    private HashMap<Image, Boolean> usedColorImages = new HashMap<>();
    private HashMap<Image, Boolean> usedDepthImages = new HashMap<>();
    private LogicalDevice device;

    public RenderImagePool(LogicalDevice device) {
        this.device = device;
    }

    public Image acquireDepthImage(int width, int height) throws VkErrorException, IllegalClassFormatException {
        Image result = null;
        for (Map.Entry<Image, Boolean> entry : usedDepthImages.entrySet()) {
            if(!entry.getValue()){
                entry.getKey().resize(width, height);
                result = entry.getKey();
                break;
            }
        }
        if(result!=null){
            usedDepthImages.put(result, true);
            return result;
        }
        result = createDepthImage(width, height);
        usedDepthImages.put(result, true);
        return result;
    }
    public Image acquireColorImage(int width, int height) throws VkErrorException, IllegalClassFormatException {
        Image result = null;
        for (Map.Entry<Image, Boolean> entry : usedColorImages.entrySet()) {
            if(!entry.getValue()){
                entry.getKey().resize(width, height);
                result = entry.getKey();
                break;
            }
        }
        if(result!=null){
            usedColorImages.put(result, true);
            return result;
        }
        result = createDepthImage(width, height);
        usedColorImages.put(result, true);
        return result;
    }

    public void releaseDepthImage(Image depthImage){
        usedDepthImages.put(depthImage, false);
    }

    public void releaseColorImage(Image colorImage){
        usedColorImages.put(colorImage, false);
    }

    @Override
    public void destroy() {
        destroyed = true;
        usedColorImages.keySet().forEach(Image::destroy);
        usedDepthImages.keySet().forEach(Image::destroy);
    }

    private Image createDepthImage(int width, int height) throws IllegalClassFormatException, VkErrorException {
        int format = device.findDepthFormat();
        defaultDepthCreateInfo.get().extent().width(width);
        defaultDepthCreateInfo.get().extent().height(height);
        defaultDepthViewCreateInfo.get().format(format);
        defaultDepthCreateInfo.get().format(format);
        Image result = new Image(device, defaultDepthCreateInfo);
        defaultDepthViewCreateInfo.get().image(result.getBase());
        result.createImageView(defaultDepthViewCreateInfo);
        result.transitionImageLayout(device, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
        return result;
    }

    private Image createColorImage(int width, int height) throws IllegalClassFormatException, VkErrorException {
        defaultColorCreateInfo.get().extent().width(width);
        defaultColorCreateInfo.get().extent().height(height);
        Image result = new Image(device, defaultColorCreateInfo);
        defaultColorViewCreateInfo.get().image(result.getBase());
        result.createImageView(defaultColorViewCreateInfo);
        result.transitionImageLayout(device, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
        return result;
    }

    static {
        try {
            defaultDepthCreateInfo = new LwjglObject<>(VkImageCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
        defaultDepthCreateInfo.get().sType$Default();
        defaultDepthCreateInfo.get().imageType(VK_IMAGE_TYPE_2D);
        defaultDepthCreateInfo.get().format(VK_FORMAT_D32_SFLOAT);
        defaultDepthCreateInfo.get().extent().width(800);
        defaultDepthCreateInfo.get().extent().height(600);
        defaultDepthCreateInfo.get().extent().depth(1);
        defaultDepthCreateInfo.get().arrayLayers(1);
        defaultDepthCreateInfo.get().mipLevels(1);
        defaultDepthCreateInfo.get().initialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        defaultDepthCreateInfo.get().samples(VK_SAMPLE_COUNT_1_BIT);
        defaultDepthCreateInfo.get().tiling(VK_IMAGE_TILING_OPTIMAL);
        defaultDepthCreateInfo.get().usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        defaultDepthCreateInfo.get().sharingMode(VK_SHARING_MODE_EXCLUSIVE);
    }

    static {
        try {
            defaultColorCreateInfo = new LwjglObject<>(VkImageCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
        defaultColorCreateInfo.get().sType$Default();
        defaultColorCreateInfo.get().imageType(VK_IMAGE_TYPE_2D);
        defaultColorCreateInfo.get().format(VK_FORMAT_R32G32B32A32_SFLOAT);
        defaultColorCreateInfo.get().extent().width(800);
        defaultColorCreateInfo.get().extent().height(600);
        defaultColorCreateInfo.get().extent().depth(1);
        defaultColorCreateInfo.get().arrayLayers(1);
        defaultColorCreateInfo.get().mipLevels(1);
        defaultColorCreateInfo.get().initialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        defaultColorCreateInfo.get().samples(VK_SAMPLE_COUNT_1_BIT);
        defaultColorCreateInfo.get().tiling(VK_IMAGE_TILING_OPTIMAL);
        defaultColorCreateInfo.get().usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        defaultColorCreateInfo.get().sharingMode(VK_SHARING_MODE_EXCLUSIVE);
    }

    static {
        try {
            defaultColorViewCreateInfo = new LwjglObject<>(VkImageViewCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }

        defaultColorViewCreateInfo.get().sType$Default();
        defaultColorViewCreateInfo.get().format(VK_FORMAT_R32G32B32A32_SFLOAT);
        defaultColorViewCreateInfo.get().viewType(VK_IMAGE_VIEW_TYPE_2D);
        defaultColorViewCreateInfo.get().subresourceRange().aspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
        defaultColorViewCreateInfo.get().subresourceRange().baseArrayLayer(0);
        defaultColorViewCreateInfo.get().subresourceRange().baseMipLevel(0);
        defaultColorViewCreateInfo.get().subresourceRange().levelCount(1);
        defaultColorViewCreateInfo.get().subresourceRange().layerCount(1);
    }

    static {
        try {
            defaultDepthViewCreateInfo = new LwjglObject<>(VkImageViewCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
        defaultDepthViewCreateInfo.get().sType$Default();
        defaultDepthViewCreateInfo.get().format(VK_FORMAT_D32_SFLOAT);
        defaultDepthViewCreateInfo.get().viewType(VK_IMAGE_VIEW_TYPE_2D);
        defaultDepthViewCreateInfo.get().subresourceRange().aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT);
        defaultDepthViewCreateInfo.get().subresourceRange().baseArrayLayer(0);
        defaultDepthViewCreateInfo.get().subresourceRange().baseMipLevel(0);
        defaultDepthViewCreateInfo.get().subresourceRange().levelCount(1);
        defaultDepthViewCreateInfo.get().subresourceRange().layerCount(1);
    }
}
