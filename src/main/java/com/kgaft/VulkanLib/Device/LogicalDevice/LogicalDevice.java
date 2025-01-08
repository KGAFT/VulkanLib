package com.kgaft.VulkanLib.Device.LogicalDevice;

import com.kgaft.VulkanLib.Device.DeviceBuilder;
import com.kgaft.VulkanLib.Device.PhysicalDevice.DeviceSuitabilityResults;
import com.kgaft.VulkanLib.Device.PhysicalDevice.PhysicalDevice;
import com.kgaft.VulkanLib.Device.PhysicalDevice.QueueFamilyInfo;
import com.kgaft.VulkanLib.Instance.Instance;
import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.StringByteBuffer;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.PointerBuffer;
import org.lwjgl.system.MemoryStack;
import org.lwjgl.vulkan.*;

import static org.lwjgl.vulkan.VK13.*;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.FloatBuffer;
import java.util.*;
import java.util.stream.IntStream;

public class LogicalDevice extends DestroyableObject {
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
            if(builder.isRequireGraphicsSupport()){

            }
            if (builder.isRayTracingSupport()) {
                features.get().shaderInt64(true);
                newFeatures.get().bufferDeviceAddress(true);
                accelerationStructureFeatures.get().accelerationStructure(true);
                newFeatures.get().pNext(accelerationStructureFeatures.get().address());
                accelerationStructureFeatures.get().pNext(rayTracingPipelineFeaturesKhr.get().address());
                newFeatures.get().runtimeDescriptorArray(true);
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
            VkDeviceCreateInfo createInfo = VkDeviceCreateInfo.calloc();
            createInfo.sType$Default();
            extensionsBuffer.rewind();
            createInfo.pQueueCreateInfos(queueCreateInfos.get());
            createInfo.ppEnabledExtensionNames(extensionsBuffer);
            createInfo.ppEnabledLayerNames(layersBuf);
            createInfo.pEnabledFeatures(features.get());
            features.get().samplerAnisotropy(true);
            createInfo.pNext(dynamicRenderingFeatures.get().address());

            PointerBuffer pb = MemoryStack.stackPush().pointers(VK_NULL_HANDLE);
            int status = vkCreateDevice(device.getBase(), createInfo, null, pb);

            if (status != VK_SUCCESS) {
                throw new VkErrorException("Failed to create device", status);
            }
            this.base = device;
            this.device = new VkDevice(pb.get(0), device.getBase(), createInfo);
            extensionsBuffer.free();
            layersBuf.free();
            for (QueueFamilyInfo element : suitabilityResults.queueFamilyInfos) {
                vkGetDeviceQueue(this.device, element.index, 0, pb);
                try {
                    queues.add(new LogicalQueue(new VkQueue(pb.get(0), this.device), this.device, element.supportPresentation, element.properties.queueFlags(), element.index));
                } catch (VkErrorException e) {
                    e.printStackTrace();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    private PhysicalDevice base;
    private VkDevice device;
    private LwjglObject<VkDeviceQueueCreateInfo.Buffer> queueCreateInfos;
    private FloatBuffer queuePriority;
    private List<LogicalQueue> queues = new ArrayList<>();


    public LogicalQueue getQueueByType(int queueType){
        Optional<LogicalQueue> queue = queue = queues.stream().filter(element-> (element.getQueueType() &queueType)>0).findFirst();
        if(queue.isEmpty()){
            throw new RuntimeException("Failed to find suitable queue");
        }
        return queue.get();
    }

    public LogicalQueue getPresentQueue(){
        Optional<LogicalQueue> queue = queues.stream().filter(element-> element.isSupportPresentation()).findFirst();
        if(queue.isPresent()){
            return queue.get();
        }
        throw new RuntimeException("Failed to find suitable queue");
    }

    public int findDepthFormat(){
        ArrayList<Integer> candidates = new ArrayList<>();
        candidates.add(VK_FORMAT_D32_SFLOAT);
        candidates.add(VK_FORMAT_D32_SFLOAT_S8_UINT);
        candidates.add(VK_FORMAT_D24_UNORM_S8_UINT);
        candidates.add(VK_FORMAT_D16_UNORM);
        candidates.add(VK_FORMAT_D16_UNORM_S8_UINT);
        return findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    public int findMemoryType(int typeFilter, int properties){

        try {
            LwjglObject<VkPhysicalDeviceMemoryProperties> memProperties = new LwjglObject<>(VkPhysicalDeviceMemoryProperties.class);
            vkGetPhysicalDeviceMemoryProperties(base.getBase(), memProperties.get());
            OptionalInt res = IntStream.range(0, memProperties.get().memoryTypeCount())
                    .filter(i -> (typeFilter & (int) (1 << i)) > 0)
                    .filter(i->(memProperties.get().memoryTypes(i).propertyFlags()&properties)== properties).findFirst();
            if(res.isPresent()){
                return res.getAsInt();
            }

            return -1;

        } catch (IllegalClassFormatException e) {
            return -1;
        }

    }


    public int getQueuesAmount(){
        return queues.size();
    }

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

    public VkDevice getDevice() {
        return device;
    }

    public PhysicalDevice getBase() {
        return base;
    }

    public int findSupportedFormat(List<Integer> candidates, int tiling, int features) {
        try{
            for (int format : candidates) {
                LwjglObject<VkFormatProperties> props = new LwjglObject<>(VkFormatProperties.class);
                vkGetPhysicalDeviceFormatProperties(base.getBase(), format, props.get());

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.get().linearTilingFeatures() & features) == features) {
                    return format;
                } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.get().optimalTilingFeatures() & features) == features) {
                    return format;
                }
            }
            throw new RuntimeException("failed to find supported format!");
        }catch (Exception e){
            e.printStackTrace();
        }
        return -1;
    }

    @Override
    public void destroy() {
        destroyed = true;
        queues.forEach(LogicalQueue::destroy);
        vkDestroyDevice(device, null);
    }
}
