using Silk.NET.Vulkan;
using VulkanLib.Device.LogicalDevice;

namespace VulkanLib.Device.Image;

public class VulImageView
{
    public VulImageView(ImageCreateInfo parentCreateInfo, ImageView imageView, VulLogicalDevice device, ImageViewCreateInfo createInfo)
    {
        
    }
    
    public ImageCreateInfo parentInfo;
    private VulLogicalDevice device;
    public ImageView viewBase;
    public ImageViewCreateInfo createInfo;

    
}