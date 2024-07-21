package com.kgaft.VulkanLib.Instance;

import com.kgaft.VulkanLib.Instance.InstanceLogger.IVulkanLoggerCallback;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
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

public class InstanceBuilder extends DestroyableObject {
    VkApplicationInfo applicationInfo = VkApplicationInfo.calloc();
    VkInstanceCreateInfo createInfo = VkInstanceCreateInfo.calloc();
    private ArrayList<String> extensionNames = new ArrayList<>();
    protected ArrayList<String> layersNames = new ArrayList<>();
    private PointerBuffer layersNamePB = null;
    private PointerBuffer extensionsNamesPB = null;
    protected List<IVulkanLoggerCallback> loggerCallbacks = new ArrayList<>();
    protected boolean debugEnabled = false;
    protected boolean presentEnabled = false;


    private boolean createInfoUsed = false;

    public InstanceBuilder() throws IllegalClassFormatException {
        applicationInfo.sType$Default();
        applicationInfo.apiVersion(VK_API_VERSION_1_3);
        createInfo.pApplicationInfo(applicationInfo);
        createInfo.sType$Default();

    }

    public void setApplicationVersion(int major, int minor, int patch) {
        applicationInfo.applicationVersion(VK_MAKE_VERSION(major, minor, patch));
    }

    public void setApplicationName(String appName) {
        applicationInfo.pApplicationName(new StringByteBuffer(appName).getDataBuffer());
    }

    public void setEngineName(String engineName) {
        applicationInfo.pEngineName(new StringByteBuffer(engineName).getDataBuffer());
    }

    public void setEngineVersion(int major, int minor, int patch) {
        applicationInfo.engineVersion(VK_MAKE_VERSION(major, minor, patch));
    }

    public void addStartingVulkanLoggerCallback(IVulkanLoggerCallback callback){
        if(debugEnabled){
            loggerCallbacks.add(callback);
        } else{
            throw new RuntimeException("Error: you cannot add logger callbacks when debugging is disabled!");
        }
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
        createInfo.ppEnabledExtensionNames(extensionsNamesPB);
        createInfo.ppEnabledLayerNames(layersNamePB);
        createInfoUsed = true;
        return createInfo;
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

    @Override
    public void destroy() {
        destroyed = true;
        if(!createInfoUsed){
            createInfo.free();
            applicationInfo.free();
        }
    }
}
