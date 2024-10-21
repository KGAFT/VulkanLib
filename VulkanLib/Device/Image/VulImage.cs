using Silk.NET.Vulkan;
using VulkanLib.Device.LogicalDevice;
using VulkanLib.ObjectManagement;

namespace VulkanLib.Device.Image;

public class VulImage : DestroyableObject
{
    public VulImage(VulLogicalDevice device, Silk.NET.Vulkan.Image imageBase)
    {
        this.device = device;
        this.imageBase = imageBase;
        this.castCreated = true;
    }

    public VulImage(VulLogicalDevice device, ImageCreateInfo createInfo)
    {
        this.device = device;
    }

    public VulImage();

    private VulLogicalDevice device;
    private Silk.NET.Vulkan.Image imageBase;
    private ImageCreateInfo imageInfo;
    private ImageView imageViews;
    private DeviceMemory imageMemory;
    private bool castCreated = false;
}