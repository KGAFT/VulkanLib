package com.kgaft.VulkanLib.Shader;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VK13;
import org.lwjgl.vulkan.VkPipelineShaderStageCreateInfo;

public class Shader extends DestroyableObject {

    public Shader(LwjglObject<VkPipelineShaderStageCreateInfo.Buffer> stages, LogicalDevice device) {
        this.stages = stages;
        this.device = device;
    }

    private LwjglObject<VkPipelineShaderStageCreateInfo.Buffer> stages;
    private LogicalDevice device;

    public LwjglObject<VkPipelineShaderStageCreateInfo.Buffer> getStages() {
        return stages;
    }

    @Override
    public void destroy() {
        destroyed = true;
        stages.get().rewind();
        stages.get().forEach(element->{
            VK13.vkDestroyShaderModule(device.getDevice(), element.module(), null);
        });
    }
}
