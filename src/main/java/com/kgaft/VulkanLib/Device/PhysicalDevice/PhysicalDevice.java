package com.kgaft.VulkanLib.Device.PhysicalDevice;

import com.kgaft.VulkanLib.Instance.Instance;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.vulkan.*;

import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.vulkan.VK13.*;

public class PhysicalDevice {
    private static List<PhysicalDevice> physicalDevices = new ArrayList<>();

    public static List<PhysicalDevice> getPhysicalDevices(Instance instance) throws VkErrorException {
        if (physicalDevices.isEmpty()) {
            int[] physicalDevicesCount = new int[1];
            vkEnumeratePhysicalDevices(instance.getInstance(), physicalDevicesCount, null);
            PointerBuffer pb = PointerBuffer.allocateDirect(physicalDevicesCount[0]);
            vkEnumeratePhysicalDevices(instance.getInstance(), physicalDevicesCount, pb);
            while (pb.hasRemaining()) {
                VkPhysicalDevice device = new VkPhysicalDevice(pb.get(), instance.getInstance());
                physicalDevices.add(new PhysicalDevice(device));
            }
        }
        return physicalDevices;
    }

    private VkPhysicalDevice base;
    private LwjglObject<VkPhysicalDeviceProperties> properties;
    private LwjglObject<VkPhysicalDeviceFeatures> features;
    private LwjglObject<VkExtensionProperties.Buffer> extensionProperties;
    private LwjglObject<VkQueueFamilyProperties.Buffer> queueProperties;
    private LwjglObject<VkPhysicalDeviceRayTracingPipelinePropertiesKHR> rayTracingPipelineProperties;

    public PhysicalDevice(VkPhysicalDevice base) throws VkErrorException {
        this.base = base;
        init(base);
    }

    public PhysicalDevice() {

    }

    public void init(VkPhysicalDevice device) throws  VkErrorException {
        try{
            int[] propertyCount = new int[1];
            int res;
            res = vkEnumerateDeviceExtensionProperties(device, (CharSequence) null, propertyCount, null);

            if (res != VK_SUCCESS) {
                throw new VkErrorException("Failed to find any extensions ", res);
            }
            extensionProperties = new LwjglObject<>(VkExtensionProperties.class, VkExtensionProperties.Buffer.class, propertyCount[0]);
            res = vkEnumerateDeviceExtensionProperties(device, (CharSequence) null, propertyCount, extensionProperties.get());
            if (res != VK_SUCCESS) {
                throw new VkErrorException("Failed to find any extensions ", res);
            }
            features = new LwjglObject<>(VkPhysicalDeviceFeatures.class);
            vkGetPhysicalDeviceFeatures(device, features.get());
            properties = new LwjglObject<>(VkPhysicalDeviceProperties.class);
            vkGetPhysicalDeviceProperties(device, properties.get());
            int[] queueFamilyCount = new int[1];
            vkGetPhysicalDeviceQueueFamilyProperties(device, queueFamilyCount, null);
            queueProperties = new LwjglObject<>(VkQueueFamilyProperties.class, VkQueueFamilyProperties.Buffer.class, queueFamilyCount[0]);
            vkGetPhysicalDeviceQueueFamilyProperties(device, queueFamilyCount, queueProperties.get());
            LwjglObject<VkPhysicalDeviceProperties2> prop2 = new LwjglObject<>(VkPhysicalDeviceProperties2.class);
            rayTracingPipelineProperties = new LwjglObject<>(VkPhysicalDeviceRayTracingPipelinePropertiesKHR.class);
            prop2.get().pNext(rayTracingPipelineProperties.get());
            prop2.get().sType$Default();
            rayTracingPipelineProperties.get().sType$Default();
            vkGetPhysicalDeviceProperties2(device, prop2.get());
        }catch (Exception e){
            e.printStackTrace();
        }

    }

    public VkPhysicalDevice getBase() {
        return base;
    }

    public LwjglObject<VkPhysicalDeviceProperties> getProperties() {
        return properties;
    }

    public LwjglObject<VkPhysicalDeviceFeatures> getFeatures() {
        return features;
    }

    public LwjglObject<VkExtensionProperties.Buffer> getExtensionProperties() {
        return extensionProperties;
    }

    public LwjglObject<VkQueueFamilyProperties.Buffer> getQueueProperties() {
        return queueProperties;
    }

    public LwjglObject<VkPhysicalDeviceRayTracingPipelinePropertiesKHR> getRayTracingPipelineProperties() {
        return rayTracingPipelineProperties;
    }

}
