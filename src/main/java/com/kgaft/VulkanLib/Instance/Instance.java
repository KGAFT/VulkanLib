package com.kgaft.VulkanLib.Instance;

import com.kgaft.VulkanLib.Instance.InstanceLogger.InstanceLogger;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.VerboseUtil;
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
    private InstanceLogger logger;
    private List<String> enabledLayers = new ArrayList<>();
    public Instance(InstanceBuilder builder){
        PointerBuffer pb = PointerBuffer.allocateDirect(1);
        VkInstanceCreateInfo createInfo = builder.getCreateInfo();
        if(builder.debugEnabled){
            logger = new InstanceLogger();
            logger.describeLogger(createInfo);
            builder.loggerCallbacks.forEach(logger::registerCallback);
        }
        int res = vkCreateInstance(createInfo, null, pb);
        if(res !=VK_SUCCESS){
            VerboseUtil.printVkErrorToString(res);
            throw new RuntimeException("Failed to create instance");
        }
        this.instance = new VkInstance(pb.get(), createInfo);
        this.enabledLayers = builder.layersNames;
        logger.init(instance);
        pb.free();
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
        vkDestroyInstance(instance, null);
    }
}
