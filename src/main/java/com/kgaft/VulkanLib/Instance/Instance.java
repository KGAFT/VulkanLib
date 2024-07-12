package com.kgaft.VulkanLib.Instance;

import com.kgaft.VulkanLib.Utils.VerboseUtil;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkInstance;

import static org.lwjgl.vulkan.VK13.VK_SUCCESS;
import static org.lwjgl.vulkan.VK13.vkCreateInstance;

public class Instance {
    private VkInstance instance;
    public Instance(InstanceBuilder builder){
        PointerBuffer pb = PointerBuffer.allocateDirect(1);
        int res = vkCreateInstance(builder.getCreateInfo(), null, pb);
        if(res !=VK_SUCCESS){
            VerboseUtil.printVkErrorToString(res);
            throw new RuntimeException("Failed to create instance");
        }
        pb.free();
    }
}
