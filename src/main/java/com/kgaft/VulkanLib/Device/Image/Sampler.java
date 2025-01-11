package com.kgaft.VulkanLib.Device.Image;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.VK13;
import org.lwjgl.vulkan.VkSamplerCreateInfo;

import java.lang.instrument.IllegalClassFormatException;

import static org.lwjgl.vulkan.VK13.*;

public class Sampler extends DestroyableObject {
    private static SeriesObject<VkSamplerCreateInfo> createInfos;

    static {
        try {
            createInfos = new SeriesObject<VkSamplerCreateInfo>(VkSamplerCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }

    public static LwjglObject<VkSamplerCreateInfo> getDefaultCreateInfo(LogicalDevice device) throws IllegalClassFormatException {
        LwjglObject<VkSamplerCreateInfo> samplerInfo = new LwjglObject<>(VkSamplerCreateInfo.class);
        samplerInfo.get().sType$Default();
        samplerInfo.get().magFilter(VK_FILTER_LINEAR);
        samplerInfo.get().minFilter(VK_FILTER_LINEAR);
        samplerInfo.get().addressModeU(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        samplerInfo.get().addressModeV(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        samplerInfo.get().addressModeW(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        samplerInfo.get().anisotropyEnable(true);
        samplerInfo.get().maxAnisotropy(device.getBase().getProperties().get().limits().maxSamplerAnisotropy());
        samplerInfo.get().borderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK);
        samplerInfo.get().unnormalizedCoordinates(false);
        samplerInfo.get().compareEnable(false);
        samplerInfo.get().compareOp(VK_COMPARE_OP_ALWAYS);
        samplerInfo.get().mipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
        return samplerInfo;
    }

    public Sampler(LogicalDevice device) throws VkErrorException {
        VkSamplerCreateInfo samplerInfo = createInfos.acquireObject();
        samplerInfo.sType$Default();
        samplerInfo.magFilter(VK_FILTER_LINEAR);
        samplerInfo.minFilter(VK_FILTER_LINEAR);
        samplerInfo.addressModeU(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        samplerInfo.addressModeV(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        samplerInfo.addressModeW(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        samplerInfo.anisotropyEnable(true);
        samplerInfo.maxAnisotropy(device.getBase().getProperties().get().limits().maxSamplerAnisotropy());
        samplerInfo.borderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK);
        samplerInfo.unnormalizedCoordinates(false);
        samplerInfo.compareEnable(false);
        samplerInfo.compareOp(VK_COMPARE_OP_ALWAYS);
        samplerInfo.mipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
        long[] res = new long[1];
        VkErrorException.checkVkStatus("Failed to create sampler: ", vkCreateSampler(device.getDevice(), samplerInfo, null, res));
        this.sampler = res[0];
        this.device = device;

        createInfos.releaseObjectInstance(samplerInfo);
    }

    public Sampler(LogicalDevice device, LwjglObject<VkSamplerCreateInfo> createInfo) throws VkErrorException {
        long[] res = new long[1];
        VkErrorException.checkVkStatus("Failed to create sampler: ", vkCreateSampler(device.getDevice(), createInfo.get(), null, res));
        this.sampler = res[0];
        this.device = device;
    }

    private long sampler;
    private LogicalDevice device;

    public long getSampler() {
        return sampler;
    }

    @Override
    public synchronized void destroy() {
        super.destroy();
        vkDestroySampler(device.getDevice(), sampler, null);
    }
}
