package com.kgaft.VulkanLib.Instance;

import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.StringByteBuffer;
import org.lwjgl.PointerBuffer;
import org.lwjgl.system.MemoryStack;
import org.lwjgl.vulkan.VkApplicationInfo;
import org.lwjgl.vulkan.VkInstanceCreateInfo;

import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import static org.lwjgl.glfw.GLFWVulkan.glfwGetRequiredInstanceExtensions;
import static org.lwjgl.vulkan.VK13.*;

public class InstanceBuilder {
    private LwjglObject<VkApplicationInfo> applicationInfo = new LwjglObject<>(VkApplicationInfo.class);
    private LwjglObject<VkInstanceCreateInfo> createInfo = new LwjglObject<>(VkInstanceCreateInfo.class);
    private ArrayList<String> extensionNames = new ArrayList<>();
    private ArrayList<String> layersNames = new ArrayList<>();
    private PointerBuffer layersNamePB = null;
    private PointerBuffer extensionsNamesPB = null;
    protected boolean debugEnabled = false;
    protected boolean presentEnabled = false;
    public InstanceBuilder() throws IllegalClassFormatException {
        applicationInfo.get().sType$Default();
        applicationInfo.get().apiVersion(VK_API_VERSION_1_3);
        createInfo.get().sType$Default();
        createInfo.get().pApplicationInfo(applicationInfo.get());


    }

    public void setApplicationVersion(int major, int minor, int patch) {
        applicationInfo.get().applicationVersion(VK_MAKE_VERSION(major, minor, patch));
    }

    public void setApplicationName(String appName) {
        applicationInfo.get().pApplicationName(new StringByteBuffer(appName).getDataBuffer());
    }

    public void setEngineName(String engineName) {
        applicationInfo.get().pEngineName(new StringByteBuffer(engineName).getDataBuffer());
    }

    public void setEngineVersion(int major, int minor, int patch) {
        applicationInfo.get().engineVersion(VK_MAKE_VERSION(major, minor, patch));
    }

    public void addExtension(String extensionName) {
        extensionNames.add(extensionName);
    }

    public void addLayer(String layerName) {
        layersNames.add(layerName);
    }

    public void addLayers(Collection<String> layers) {
        layersNames.addAll(layers);
    }


    public void addExtensions(Collection<String> extensions) {
        extensionNames.addAll(extensions);
    }
    //Call for glfw init firstly
    public void presetForPresent(){
        extensionNames.add("VK_EXT_swapchain_colorspace");
        presentEnabled = true;
    }

    public void presetForDebug() {
        addLayer("VK_LAYER_KHRONOS_validation");
        addExtension("VK_EXT_debug_utils");
        debugEnabled = true;
    }

    protected VkInstanceCreateInfo getCreateInfo() {
        packExtensionsToPB();
        packLayersToPB();
        extensionsNamesPB.rewind();
        layersNamePB.rewind();
        createInfo.get().ppEnabledExtensionNames(extensionsNamesPB);
        createInfo.get().ppEnabledLayerNames(layersNamePB);
        return createInfo.get();
    }

    private void packLayersToPB() {
        if (!layersNames.isEmpty()) {
            if (layersNamePB == null) {
                layersNamePB = PointerBuffer.allocateDirect(layersNames.size());
            }
            if (layersNamePB.capacity() != layersNames.size()) {
                layersNamePB.free();
                layersNamePB = PointerBuffer.allocateDirect(layersNames.size());
            }
            if (layersNamePB.hasRemaining()) {
                layersNames.forEach(element -> layersNamePB.put(new StringByteBuffer(element).getDataBuffer()));
            }
        }
    }

    private void packExtensionsToPB() {
        if (!extensionNames.isEmpty() || presentEnabled) {
            int extensionCount = extensionNames.size()+(presentEnabled?glfwGetRequiredInstanceExtensions().capacity():0);
            if (extensionsNamesPB == null) {
                extensionsNamesPB = PointerBuffer.allocateDirect(extensionCount);
            }
            if (extensionsNamesPB.capacity() != extensionCount) {
                extensionsNamesPB.free();
                extensionsNamesPB = PointerBuffer.allocateDirect(extensionCount);
            }
            if (extensionsNamesPB.hasRemaining()) {
                extensionNames.forEach(element -> extensionsNamesPB.put(new StringByteBuffer(element).getDataBuffer()));
                if(presentEnabled){
                    PointerBuffer glfwExtensions = glfwGetRequiredInstanceExtensions();
                    extensionsNamesPB.put(glfwExtensions);
                }
            }
        }
    }
}
