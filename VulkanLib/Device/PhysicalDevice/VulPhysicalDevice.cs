using VulkanLib.ObjectManagement;

namespace VulkanLib.Device.PhysicalDevice;

using Silk.NET.Vulkan;

public class VulPhysicalDevice
{
    private static List<VulPhysicalDevice> physicalDevices = new();
    public static List<VulPhysicalDevice> getDevices(VulInstance instance)
    {
        if(physicalDevices.Count != 0)
            return physicalDevices;
        foreach (var physicalDevice in Vk.GetApi().GetPhysicalDevices(instance.getBase()))
        {
            physicalDevices.Add(new(physicalDevice));
        }
        return physicalDevices;
    }
    public unsafe VulPhysicalDevice(PhysicalDevice device)
    {
        this.device = device;
        uint extentionsCount = 0;
        VulResultException.checkResult("Failed to obtain device info: ",
            Vk.GetApi().EnumerateDeviceExtensionProperties(device, (byte*)null, ref extentionsCount, null));
        
        var availableExtensions = new ExtensionProperties[extentionsCount];
        fixed (ExtensionProperties* availableExtensionsPtr = availableExtensions)
        {
            Vk.GetApi().EnumerateDeviceExtensionProperties(device, (byte*)null, ref extentionsCount, availableExtensionsPtr);
        }
        foreach (var availableExtension in availableExtensions)
        {
            extensionProperties.Add(availableExtension);
        }
        Vk.GetApi().GetPhysicalDeviceFeatures(device, out features);
        Vk.GetApi().GetPhysicalDeviceProperties(device, out properties);

        uint queueFamilityCount = 0;
        Vk.GetApi().GetPhysicalDeviceQueueFamilyProperties(device, ref queueFamilityCount, null);

        var queueFamilies = new QueueFamilyProperties[queueFamilityCount];
        fixed (QueueFamilyProperties* queueFamiliesPtr = queueFamilies)
        {
            Vk.GetApi().GetPhysicalDeviceQueueFamilyProperties(device, ref queueFamilityCount, queueFamiliesPtr);
        }
        foreach (var queueFamilyPropertiese in queueFamilies)
        {
            queueProperties.Add(queueFamilyPropertiese);
        }
        PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelinePropertiesKhr = new();
        PhysicalDeviceProperties2 properties2 = new();
        properties2.PNext = &rayTracingPipelinePropertiesKhr;
        this.rayTracingPipelinePropertiesKhr = rayTracingPipelinePropertiesKhr;
    }
    private PhysicalDevice device;
    private PhysicalDeviceProperties properties = new ();
    private PhysicalDeviceFeatures features = new();
    private List<ExtensionProperties> extensionProperties = new();
    private List<QueueFamilyProperties> queueProperties = new();
    private PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelinePropertiesKhr;


    public PhysicalDevice getBase() => device;

    public PhysicalDeviceProperties getProperties() => properties;

    public PhysicalDeviceFeatures getFeatures() => features;

    public List<ExtensionProperties> getExtensionProperties() => extensionProperties;

    public List<QueueFamilyProperties> getQueueProperties() => queueProperties;

    public PhysicalDeviceRayTracingPipelinePropertiesKHR getRayTracingPipelinePropertiesKhr() => rayTracingPipelinePropertiesKhr;
}