package com.kgaft.VulkanLib.Device.Buffer;

import com.kgaft.VulkanLib.Utils.MemoryUtils;
import org.lwjgl.vulkan.VkCommandBuffer;

import java.nio.ByteBuffer;

import static org.lwjgl.vulkan.VK13.*;

public class PushConstant {
    public PushConstant(long size, long pipelineLayout){
        this.size = size;
        this.pipelineLayout = pipelineLayout;
        data = ByteBuffer.allocateDirect((int) size);
    }
    private long size;
    private long pipelineLayout;
    private ByteBuffer data;

    public void bind(VkCommandBuffer cmd, int bindPoint){
        vkCmdPushConstants(cmd, pipelineLayout, bindPoint, 0, data);
    }

    public void writeData(ByteBuffer bb){
        MemoryUtils.memcpy(data, bb, size);
        data = data.rewind();
    }

}
