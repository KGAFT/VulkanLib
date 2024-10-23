using Silk.NET.Core.Native;
using VulkanLib.Device.PhysicalDevice;
using VulkanLib.ObjectManagement;

namespace VulkanLib.Device.LogicalDevice;

using Silk.NET.Vulkan;

public class VulLogicalDevice : DestroyableObject
{
    public unsafe VulLogicalDevice(VulInstance instance, VulPhysicalDevice device, VulDeviceBuilder builder,
        DeviceSuitabilityResults results)
    {
        float priority = 1.0f;
        sanitizeQueueCreateInfos(results, &priority);
        PhysicalDeviceFeatures features = new();
        features.ShaderInt64 = true;
        PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = new();
        PhysicalDeviceAccelerationStructureFeaturesKHR accelStructure = new();
        dynamicRenderingFeature.SType = StructureType.PhysicalDeviceDynamicRenderingFeaturesKhr;
        accelStructure.SType = StructureType.PhysicalDeviceAccelerationStructureFeaturesKhr;
        dynamicRenderingFeature.DynamicRendering = true;

        PhysicalDeviceVulkan12Features newFeatures = new();
        newFeatures.SType = StructureType.PhysicalDeviceVulkan12Features;
        newFeatures.BufferDeviceAddress = true;
        newFeatures.DescriptorIndexing = true;
        newFeatures.RuntimeDescriptorArray = true;
        newFeatures.ShaderSampledImageArrayNonUniformIndexing = true;
        newFeatures.UniformAndStorageBuffer8BitAccess = true;
        newFeatures.StorageBuffer8BitAccess = true;
        newFeatures.ShaderInt8 = true;
        features.FragmentStoresAndAtomics = true;
        dynamicRenderingFeature.PNext = &newFeatures;
        PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKhr = new();
        rayTracingPipelineFeaturesKhr.SType = StructureType.PhysicalDeviceRayTracingPipelineFeaturesKhr;
        rayTracingPipelineFeaturesKhr.RayTracingPipeline = true;
        if (builder.rayTracingSupport1())
        {
            accelStructure.AccelerationStructure = true;
            newFeatures.PNext = &accelStructure;
            accelStructure.PNext = &rayTracingPipelineFeaturesKhr;
        }

        var queuesInfo = queueCreateInfos.ToArray();
        fixed (DeviceQueueCreateInfo* pQueuesInfo = queuesInfo)
        {
            DeviceCreateInfo deviceInfo = new DeviceCreateInfo();
            deviceInfo.SType = StructureType.DeviceCreateInfo;
            deviceInfo.EnabledExtensionCount = (uint)builder.getRequestExtensions().Count;
            deviceInfo.EnabledLayerCount = (uint)instance.getEnabledLayers().Count;
            deviceInfo.PpEnabledLayerNames = (byte**)SilkMarshal.StringArrayToPtr(instance.getEnabledLayers());
            deviceInfo.PpEnabledExtensionNames = (byte**)SilkMarshal.StringArrayToPtr(builder.getRequestExtensions());
            deviceInfo.PEnabledFeatures = &features;
            deviceInfo.PQueueCreateInfos = pQueuesInfo;
            deviceInfo.QueueCreateInfoCount = (uint)queuesInfo.Length;
            deviceInfo.PNext = &dynamicRenderingFeature;

            VulResultException.checkResult("Failed to create device: ",
                Vk.GetApi().CreateDevice(device.getBase(), in deviceInfo, null, out this.device));
        }

        foreach (var deviceQueueCreateInfo in results.queuesInfo)
        {
            Queue queue;
            Vk.GetApi().GetDeviceQueue(this.device, deviceQueueCreateInfo.index, 0, out queue);
            queues.Add(new(queue, this.device, deviceQueueCreateInfo.supportPresentation,
                deviceQueueCreateInfo.properties.QueueFlags, deviceQueueCreateInfo.index));
        }
        this.instance = instance;
    }

    private Device device;
    private List<VulLogicalQueue> queues = new();
    private VulPhysicalDevice baseDevice;
    private PhysicalDeviceMemoryProperties memProperties;
    private bool memoryPropertiesPopulated = false;
    private List<DeviceQueueCreateInfo> queueCreateInfos = new();
    private VulInstance instance;
    public Device getDevice()
    {
        return device;
    }

    public VulInstance getInstance() => instance;
    
    public VulPhysicalDevice getBaseDevice()
    {
        return baseDevice;
    }

    public VulLogicalQueue getQueueByType(QueueFlags queueType)
    {
        foreach (var vulLogicalQueue in queues)
        {
            if ((vulLogicalQueue.getQueueType() & queueType) > 0)
            {
                return vulLogicalQueue;
            }
        }

        throw new Exception("Failed to find suitable queue");
    }

    public VulLogicalQueue getPresentQueue()
    {
        foreach (var vulLogicalQueue in queues)
        {
            if (vulLogicalQueue.isSupportPresentation())
                return vulLogicalQueue;
        }

        throw new Exception("Failed to find suitable queue");
    }
    
    
    public Format findDepthFormat()
    {
        return findSupportedFormat(
            new() { Format.D32Sfloat, Format.D32SfloatS8Uint, Format.D24UnormS8Uint },
            ImageTiling.Optimal,
            FormatFeatureFlags.DepthStencilAttachmentBit);
    }

    public uint findMemoryType(uint typeFilter, MemoryPropertyFlags properties)
    {
        if (!memoryPropertiesPopulated)
        {
            Vk.GetApi().GetPhysicalDeviceMemoryProperties(baseDevice.getBase());
            memoryPropertiesPopulated = true;
        }

        for (int i = 0; i < memProperties.MemoryTypeCount; i++)
        {
            if ((typeFilter & 1) == 1)
            {
                if ((memProperties.MemoryTypes[i].PropertyFlags & properties) == properties)
                {
                    return (uint)i;
                }
            }

            typeFilter >>= 1;
        }

        throw new Exception("Failed to find suitable memory type!");
    }


    public Format findSupportedFormat(List<Format> candidates, ImageTiling tiling, FormatFeatureFlags features)
    {
        foreach (var candidate in candidates)
        {
            var properties = Vk.GetApi().GetPhysicalDeviceFormatProperties(baseDevice.getBase(), candidate);
            if (tiling == ImageTiling.Linear && (properties.LinearTilingFeatures & features) == features)
            {
                return candidate;
            }
            else if (tiling == ImageTiling.Optimal && (properties.OptimalTilingFeatures & features) == features)
            {
                return candidate;
            }
        }

        throw new Exception("Failed to find suitable format!");
    }

    public uint getQueuesAmount()
    {
        return (uint)queues.Count;
    }


    private unsafe void sanitizeQueueCreateInfos(DeviceSuitabilityResults results, float* priority)
    {
        uint counter = 0;
        foreach (var item in results.queuesInfo)
        {
            DeviceQueueCreateInfo queueCreateInfo = new(StructureType.DeviceQueueCreateInfo,
                null, 0, item.index,
                1, priority);
            queueCreateInfos.Add(queueCreateInfo);

            counter++;
        }
    }

    public unsafe override void destroy()
    {
        foreach (var vulLogicalQueue in queues)
        {
            vulLogicalQueue.destroy();
        }

        Vk.GetApi().DestroyDevice(device, null);
    }
}