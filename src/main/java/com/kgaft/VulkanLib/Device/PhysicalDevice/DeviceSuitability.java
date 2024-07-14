package com.kgaft.VulkanLib.Device.PhysicalDevice;

import com.kgaft.VulkanLib.Device.DeviceBuilder;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.VkExtensionProperties;
import org.lwjgl.vulkan.VkQueueFamilyProperties;
import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRSurface.*;
public class DeviceSuitability {
    public static DeviceSuitabilityResults isDeviceSuitable(DeviceBuilder builder, PhysicalDevice device) throws VkErrorException {
        for (String item : builder.getRequestExtensions()) {
            boolean found = false;
            while (device.getExtensionProperties().get().hasRemaining()) {
                VkExtensionProperties properties = device.getExtensionProperties().get().get();
                if (properties.extensionNameString().equals(item)) {
                    found = true;
                    break;
                }

            }
            device.getExtensionProperties().get().rewind();
            if (!found) {
                return null;
            }
        }
        boolean graphicsFound = false;
        boolean presentFound = false;
        boolean computeFound = false;
        int queueCounter = 0;
        QueueFamilyInfo info = new QueueFamilyInfo();
        DeviceSuitabilityResults results = new DeviceSuitabilityResults();
        while(device.getQueueProperties().get().hasRemaining()){
            VkQueueFamilyProperties item = device.getQueueProperties().get().get();
            if (builder.isRequireGraphicsSupport()) {
                if ((item.queueFlags() & VK_QUEUE_GRAPHICS_BIT)>0){
                    graphicsFound = true;
                    info = new QueueFamilyInfo(queueCounter, item, false);
                }
            }
            if (builder.isRequireComputeSupport()) {
                if ((item.queueFlags() & VK_QUEUE_COMPUTE_BIT)>0){
                    computeFound = true;
                    info = new QueueFamilyInfo(queueCounter, item, false);
                }
            }
            if (builder.isRequirePresentSupport()) {
                int[] isSurfaceSupported = new int[1];
                int status = vkGetPhysicalDeviceSurfaceSupportKHR(device.getBase(), queueCounter, builder.getSurfaceForPresentationCheck(), isSurfaceSupported);
                if(status!=VK_SUCCESS){
                    throw new VkErrorException("Error to check the surface support by device: ", status);
                }
                if (isSurfaceSupported[0]>0) {
                    presentFound = true;
                    info = new QueueFamilyInfo(queueCounter, item, true);
                }
            }
            results.queueFamilyInfos.add(info);
            queueCounter++;
            if ((graphicsFound == builder.isRequireGraphicsSupport()) && (presentFound == builder.isRequirePresentSupport()) &&
                    (computeFound == builder.isRequireComputeSupport())) {
                break;
            }

        }
        device.getQueueProperties().get().rewind();
        if (!((graphicsFound == builder.isRequireGraphicsSupport()) && (presentFound == builder.isRequirePresentSupport()) &&
                (computeFound == builder.isRequireComputeSupport()))) {
            return null;
        }
        return results;
    }
}
