package com.kgaft.VulkanLib.Instance;

import com.kgaft.VulkanLib.Instance.InstanceLogger.InstanceLogger;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.VkInstance;
import org.lwjgl.vulkan.VkInstanceCreateInfo;

import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.vulkan.VK10.vkDestroyInstance;
import static org.lwjgl.vulkan.VK13.VK_SUCCESS;
import static org.lwjgl.vulkan.VK13.vkCreateInstance;

public class Instance extends DestroyableObject {
    private VkInstance instance;
    private InstanceLogger logger = null;
    private List<String> enabledLayers = new ArrayList<>();
    public Instance(InstanceBuilder builder) throws VkErrorException {
        PointerBuffer pb = PointerBuffer.allocateDirect(1);
        VkInstanceCreateInfo createInfo = builder.getCreateInfo();
        if(builder.debugEnabled){
            logger = new InstanceLogger();
            logger.describeLogger(createInfo);
            builder.loggerCallbacks.forEach(logger::registerCallback);
        }
        int res = vkCreateInstance(createInfo, null, pb);
        if(res !=VK_SUCCESS){
            throw new VkErrorException("Failed to create instance", res);
        }
        this.instance = new VkInstance(pb.get(), createInfo);
        this.enabledLayers = builder.layersNames;
        if(builder.debugEnabled)
            logger.init(instance);

    }

    public VkInstance getInstance() {
        return instance;
    }

    public InstanceLogger getLogger() {
        return logger;
    }

    public List<String> getEnabledLayers() {
        return enabledLayers;
    }

    @Override
    public void destroy() {
        this.destroyed = true;
        if(logger!=null){
            logger.destroy();
        }
        vkDestroyInstance(instance, null);
    }
}
