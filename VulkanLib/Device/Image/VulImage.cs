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

    public  VulImage(VulLogicalDevice device, ImageCreateInfo createInfo)
    {
        initialize(device, createInfo);
    }

    private VulLogicalDevice device;
    private Silk.NET.Vulkan.Image imageBase;
    private ImageCreateInfo imageInfo;
    private DeviceMemory imageMemory;
    private bool castCreated = false;
    private List<VulImageView> imageViews = new();

    public void transitionImageLayout(VulLogicalDevice device, ImageLayout oldLayout,
        ImageLayout newLayout, ImageAspectFlags aspectFlags)
    {
        CommandBuffer commandBuffer = device.getQueueByType(QueueFlags.GraphicsBit).beginSingleTimeCommands();

        transitionImageLayout(commandBuffer, oldLayout, newLayout, aspectFlags);

        device.getQueueByType(QueueFlags.GraphicsBit).endSingleTimeCommands(commandBuffer);
    }

    public unsafe void initialize(VulLogicalDevice device, ImageCreateInfo createInfo)
    {
        Result res;
        this.device = device;
        this.imageInfo = createInfo;
        Silk.NET.Vulkan.Image tmp = new();
        VulResultException.checkResult("Failed to create image: ",
            Vk.GetApi().CreateImage(device.getDevice(), &createInfo, null, &tmp));
        this.imageBase = tmp;

        MemoryRequirements requirements;
        Vk.GetApi().GetImageMemoryRequirements(device.getDevice(), imageBase, &requirements);

        MemoryAllocateInfo allocInfo = new();
        allocInfo.SType = StructureType.MemoryAllocateInfo;
        allocInfo.AllocationSize = requirements.Size;
        allocInfo.MemoryTypeIndex =
            device.findMemoryType(requirements.MemoryTypeBits, MemoryPropertyFlags.DeviceLocalBit);
        DeviceMemory memTmp = new();
        VulResultException.checkResult("Failed to allocate image memory: ",
            Vk.GetApi().AllocateMemory(device.getDevice(), allocInfo, null, &memTmp));
        this.imageMemory = memTmp;
        Vk.GetApi().BindImageMemory(device.getDevice(), imageBase, memTmp, 0);
    }

    public unsafe VulImageView createImageView(ImageViewCreateInfo createInfo)
    {
        ImageView tmpRes = new();
        VulResultException.checkResult("Failed to create image view: ", Vk.GetApi().CreateImageView(device.getDevice(), createInfo, null, &tmpRes));
        var result = new VulImageView(imageInfo, tmpRes, device, createInfo);
        imageViews.Add(result);
        return result;
    }
    public unsafe void resize(uint width, uint height) {
        if (!castCreated) {
            destroy();
            destroyed = false;
            imageInfo.Extent = new(width, height, 1);
            if(imageInfo.InitialLayout!=ImageLayout.Undefined){
                ImageLayout tmpLayout = imageInfo.InitialLayout;
                imageInfo.InitialLayout = ImageLayout.Undefined;
                initialize(device, imageInfo);
                transitionImageLayout(device, ImageLayout.Undefined, tmpLayout, ImageAspectFlags.ColorBit);
                imageInfo.InitialLayout = tmpLayout;
            } else {
                initialize(device, imageInfo);
            }
            foreach (var item in imageViews)
            {
                item.createInfo.Image = imageBase;
                ImageView tmpRes = new();
                VulResultException.checkResult("Failed to recreate image view: ", Vk.GetApi().CreateImageView(device.getDevice(), item.createInfo, null, &tmpRes));
                item.viewBase = tmpRes;
                item.parentInfo = imageInfo;
            }
            
        }

    }
    
    public unsafe void transitionImageLayout(CommandBuffer commandBuffer, ImageLayout oldLayout, ImageLayout newLayout,
        ImageAspectFlags aspectFlags)
    {
        ImageMemoryBarrier barrier = new();
        barrier.SType = StructureType.ImageMemoryBarrier;
        barrier.OldLayout = oldLayout;
        barrier.NewLayout = newLayout;
        barrier.SrcQueueFamilyIndex = ~0U;
        barrier.DstQueueFamilyIndex = ~0U;
        barrier.Image = imageBase;
        barrier.SubresourceRange.AspectMask = aspectFlags;
        barrier.SubresourceRange.BaseMipLevel = 0;
        barrier.SubresourceRange.LevelCount = 1;
        barrier.SubresourceRange.BaseArrayLayer = 0;
        barrier.SubresourceRange.LayerCount = 1;

        PipelineStageFlags sourceStage;
        PipelineStageFlags destinationStage;

        if (oldLayout == ImageLayout.Undefined && newLayout == ImageLayout.TransferDstOptimal)
        {
            barrier.SrcAccessMask = 0;
            barrier.DstAccessMask = AccessFlags.TransferWriteBit;

            sourceStage = PipelineStageFlags.TopOfPipeBit;
            destinationStage = PipelineStageFlags.TransferBit;
        }
        else if (oldLayout == ImageLayout.TransferDstOptimal &&
                 newLayout == ImageLayout.ShaderReadOnlyOptimal)
        {
            barrier.SrcAccessMask = AccessFlags.TransferWriteBit;
            barrier.DstAccessMask = AccessFlags.ShaderReadBit | AccessFlags.ShaderWriteBit;

            sourceStage = PipelineStageFlags.TransferBit;
            destinationStage = PipelineStageFlags.FragmentShaderBit;
        }
        else
        {
            barrier.SrcAccessMask = 0;
            barrier.DstAccessMask = AccessFlags.ShaderReadBit | AccessFlags.ShaderWriteBit;

            sourceStage = PipelineStageFlags.TopOfPipeBit;
            destinationStage = PipelineStageFlags.FragmentShaderBit;
        }

        Vk.GetApi().CmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, null, 0, null, 1, barrier);
    }

    public override unsafe void destroy()
    {
        if (!castCreated) {
            foreach (var item in imageViews)
            {
                item.destroy();
            }
            Vk.GetApi().DestroyImage(device.getDevice(), imageBase, null);
            Vk.GetApi().FreeMemory(device.getDevice(), imageMemory, null);
        }
        destroyed = true;
    }
}