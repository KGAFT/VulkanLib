package com.kgaft.VulkanLib.Device.LogicalDevice;

import com.kgaft.VulkanLib.Device.DeviceBuilder;
import com.kgaft.VulkanLib.Device.PhysicalDevice.DeviceSuitabilityResults;
import com.kgaft.VulkanLib.Device.PhysicalDevice.PhysicalDevice;
import com.kgaft.VulkanLib.Instance.Instance;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.StringByteBuffer;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.system.MemoryStack;
import org.lwjgl.vulkan.*;

import static org.lwjgl.vulkan.VK13.*;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.List;

public class LogicalDevice {
    public LogicalDevice(Instance instance, PhysicalDevice device, DeviceBuilder builder, DeviceSuitabilityResults suitabilityResults) throws VkErrorException {
        try {
            try (MemoryStack stack = MemoryStack.stackPush()) {
                queuePriority = stack.callocFloat(1);
                queuePriority.put(1.0f);
                queuePriority.rewind();
            }

            sanitizeQueueCreateInfos(suitabilityResults);
            LwjglObject<VkPhysicalDeviceFeatures> features = new LwjglObject<>(VkPhysicalDeviceFeatures.class);
            LwjglObject<VkPhysicalDeviceDynamicRenderingFeaturesKHR> dynamicRenderingFeatures = new LwjglObject<>(VkPhysicalDeviceDynamicRenderingFeaturesKHR.class);
            LwjglObject<VkPhysicalDeviceAccelerationStructureFeaturesKHR> accelerationStructureFeatures = new LwjglObject<>(VkPhysicalDeviceAccelerationStructureFeaturesKHR.class);
            dynamicRenderingFeatures.get().dynamicRendering(true);
            LwjglObject<VkPhysicalDeviceVulkan12Features> newFeatures = new LwjglObject<>(VkPhysicalDeviceVulkan12Features.class);

            newFeatures.get().bufferDeviceAddress(true);
            newFeatures.get().descriptorIndexing(true);
            newFeatures.get().runtimeDescriptorArray(true);
            newFeatures.get().shaderSampledImageArrayNonUniformIndexing(true);
            newFeatures.get().uniformAndStorageBuffer8BitAccess(true);
            newFeatures.get().storageBuffer8BitAccess(true);
            newFeatures.get().shaderInt8(true);
            dynamicRenderingFeatures.get().pNext(newFeatures.get().address());
            LwjglObject<VkPhysicalDeviceRayTracingPipelineFeaturesKHR> rayTracingPipelineFeaturesKhr = new LwjglObject<>(VkPhysicalDeviceRayTracingPipelineFeaturesKHR.class);
            rayTracingPipelineFeaturesKhr.get().rayTracingPipeline(true);
            if (builder.isRayTracingSupport()) {
                accelerationStructureFeatures.get().accelerationStructure(true);
                newFeatures.get().pNext(accelerationStructureFeatures.get().address());
                accelerationStructureFeatures.get().pNext(rayTracingPipelineFeaturesKhr.get().address());

            }

            PointerBuffer layersBuf = PointerBuffer.allocateDirect(instance.getEnabledLayers().size());
            List<StringByteBuffer> layersBuffBase = new ArrayList<>();
            instance.getEnabledLayers().forEach(element -> {
                StringByteBuffer layerBuff = new StringByteBuffer(element);
                layersBuf.put(layerBuff.getDataBuffer());
                layersBuffBase.add(layerBuff);
            });
            layersBuf.rewind();
            PointerBuffer extensionsBuffer = PointerBuffer.allocateDirect(builder.getRequestExtensions().size());
            List<StringByteBuffer> extensionsBuffBase = new ArrayList<>();
            builder.getRequestExtensions().forEach(element -> {
                StringByteBuffer extBuff = new StringByteBuffer(element);
                extensionsBuffer.put(extBuff.getDataBuffer());
                extensionsBuffBase.add(extBuff);
            });
            LwjglObject<VkDeviceCreateInfo> createInfo = new LwjglObject<>(VkDeviceCreateInfo.class);

            extensionsBuffer.rewind();
            createInfo.get().pQueueCreateInfos(queueCreateInfos.get());
            createInfo.get().ppEnabledExtensionNames(extensionsBuffer);
            createInfo.get().ppEnabledLayerNames(layersBuf);
            createInfo.get().pEnabledFeatures(features.get());
            createInfo.get().pNext(dynamicRenderingFeatures.get());
            PointerBuffer pb = PointerBuffer.allocateDirect(1);
            pb.rewind();
            int status = vkCreateDevice(device.getBase(), createInfo.get(), null, pb);

            if (status != VK_SUCCESS) {
                throw new VkErrorException("Failed to create device", status);
            }
            this.base = device;
            this.device = new VkDevice(pb.get(), device.getBase(), createInfo.get());
            extensionsBuffer.free();
            layersBuf.free();
            pb.free();
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    private PhysicalDevice base;
    private VkDevice device;
    private LwjglObject<VkDeviceQueueCreateInfo.Buffer> queueCreateInfos;
    private FloatBuffer queuePriority;

    private void sanitizeQueueCreateInfos(DeviceSuitabilityResults results) throws IllegalClassFormatException {
        int counter = 0;
        queueCreateInfos = new LwjglObject<>(VkDeviceQueueCreateInfo.class, VkDeviceQueueCreateInfo.Buffer.class, results.queueFamilyInfos.size());
        while (counter < queueCreateInfos.get().capacity()) {
            queueCreateInfos.get().get(counter).sType$Default();
            queueCreateInfos.get().get(counter).queueFamilyIndex(results.queueFamilyInfos.get(counter).index);
            queueCreateInfos.get().get(counter).pQueuePriorities(queuePriority);
            counter++;
        }
        queueCreateInfos.get().rewind();
    }
}
