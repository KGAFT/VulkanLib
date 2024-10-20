using Silk.NET.Core.Native;
using VulkanLib.Device.PhysicalDevice;
using VulkanLib.ObjectManagement;

namespace VulkanLib.Device.LogicalDevice;

using Silk.NET.Vulkan;

public class VulLogicalDevice
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

        dynamicRenderingFeature.DynamicRendering = true;

        PhysicalDeviceVulkan12Features newFeatures = new();
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
            
            VulResultException.checkResult("Failed to create device: ", Vk.GetApi().CreateDevice(device.getBase(), in deviceInfo, null, out this.device));
            
        }
        foreach (var deviceQueueCreateInfo in results.queuesInfo)
        {
            Queue queue;
            Vk.GetApi().GetDeviceQueue(this.device, deviceQueueCreateInfo.index, 0, out queue);
            queues.Add(new (queue));
        }
    }

    private Device device;
    private List<VulLogicalQueue> queues;
    private VulPhysicalDevice baseDevice;
    private PhysicalDeviceMemoryProperties memProperties;
    private bool memoryPropertiesPopulated = false;
    private List<DeviceQueueCreateInfo> queueCreateInfos = new();

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
}