using Silk.NET.Vulkan;
using VulkanLib.Device.LogicalDevice;
using VulkanLib.ObjectManagement;

namespace VulkanLib.Device.Image;

public class VulImageView : DestroyableObject
{
    public VulImageView(ImageCreateInfo parentCreateInfo, ImageView imageView, VulLogicalDevice device, ImageViewCreateInfo createInfo)
    {
        this.parentInfo = parentCreateInfo;
        this.viewBase = imageView;
        this.device = device;
        this.createInfo = createInfo;
    }
    
    public ImageCreateInfo parentInfo;
    private VulLogicalDevice device;
    public ImageView viewBase;
    public ImageViewCreateInfo createInfo;

    public override unsafe void destroy()
    {
        this.destroyed = true;
        Vk.GetApi().DestroyImageView(device.getDevice(), viewBase, null);
    }
}