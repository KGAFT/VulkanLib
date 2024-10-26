using Silk.NET.Vulkan;
using Silk.NET.Vulkan.Extensions.KHR;
using VulkanLib.Device.Image;
using VulkanLib.Device.LogicalDevice;
using VulkanLib.ObjectManagement;

namespace VulkanLib.Device;

struct SwapChainSupportDetails
{
    public SurfaceCapabilitiesKHR capabilities;
    public List<SurfaceFormatKHR> formats = new();
    public List<PresentModeKHR> presentModes = new();

    public SwapChainSupportDetails()
    {
        capabilities = default;
    }
}

public class VulSwapChain : DestroyableObject
{
    public VulSwapChain(VulInstance instance, VulLogicalDevice device,  SurfaceKHR surface, uint width, uint height,
    bool enableFrameLock)
    {
           this.instance = instance;
           this.device = device;
           this.surface = surface;
           createSwapChain(width, height, enableFrameLock);
    }
    private static KhrSwapchain swapchainExtension = null;
    private VulLogicalDevice device;
    private SwapchainKHR swapchain;
    private SurfaceKHR surface;
    private PresentModeKHR presentMode;
    private Extent2D extent;
    private SurfaceFormatKHR format;
    private List<Silk.NET.Vulkan.Image> defaultImages;
    private List<VulImage> images = new();
    private List<VulImageView> imagesViews = new();
    private bool enableFrameLock = false;
    private VulInstance instance;
    
    
    public List<VulImageView> getSwapchainImageViews()  {
        return imagesViews;
    }

    public SwapchainKHR getSwapchainKhr() { return swapchain; }
    public void recreate(uint width, uint height) {
        destroy();
        destroyed = false;
        cleanUpImages();
        createSwapChain(width, height, enableFrameLock);
    }

    public void recreate(uint width, uint height, bool refreshRateLock) {
        destroy();
        destroyed = false;
        cleanUpImages();
        enableFrameLock = refreshRateLock;
        createSwapChain(width, height, refreshRateLock);
    }

    public List<VulImage> getSwapchainImages()  {
        return images;
    }
    
    private unsafe void createSwapChain(uint width, uint height, bool enableFrameLock)
    {
        SwapChainSupportDetails support = new();
        gatherSwapChainSupportDetails(out support);

        format = chooseSurfaceFormat(support.formats);
        presentMode = choosePresentMode(support.presentModes, enableFrameLock);
        extent = chooseSwapchainExtent(width, height, support.capabilities);
        uint imageCount = support.capabilities.MinImageCount + 1;
        if (support.capabilities.MaxImageCount > 0 &&
            imageCount > support.capabilities.MaxImageCount)
        {
            imageCount = support.capabilities.MaxImageCount;
        }

        SwapchainCreateInfoKHR createInfo = new(StructureType.SwapchainCreateInfoKhr,
            null, 0, surface, imageCount, format.Format,
            format.ColorSpace, extent, 1, ImageUsageFlags.ColorAttachmentBit);
        HashSet<uint> queueIndices = new HashSet<uint>()
        {
            device.getQueueByType(QueueFlags.GraphicsBit).getIndex(),
            device.getPresentQueue().getIndex()
        };
        var tmpindices = new List<uint>(queueIndices.ToArray());
        fixed (uint* pIndices = tmpindices.ToArray())
        {
            if (queueIndices.Count > 1)
            {
                createInfo.ImageSharingMode = SharingMode.Concurrent;
                createInfo.QueueFamilyIndexCount = (uint)tmpindices.Count;
                createInfo.PQueueFamilyIndices = pIndices;
            }
            else
            {
                createInfo.ImageSharingMode = SharingMode.Exclusive;
            }

            createInfo.PreTransform = support.capabilities.CurrentTransform;
            createInfo.CompositeAlpha = CompositeAlphaFlagsKHR.OpaqueBitKhr;
            createInfo.PresentMode = presentMode;
            createInfo.Clipped = true;

            if (swapchainExtension == null)
            {
                Vk.GetApi().TryGetDeviceExtension(device.getInstance().getBase(), device.getDevice(),
                    out swapchainExtension);
            }

            SwapchainKHR tmp;
            VulResultException.checkResult("Failed to create swapchain: ",
                swapchainExtension.CreateSwapchain(device.getDevice(), createInfo, null, &tmp));
            this.swapchain = tmp;
            uint imgCount = 0;
            swapchainExtension.GetSwapchainImages(device.getDevice(), swapchain, &imgCount, null);
            Silk.NET.Vulkan.Image[] tmpImages = new Silk.NET.Vulkan.Image[imgCount];
            swapchainExtension.GetSwapchainImages(device.getDevice(), swapchain, &imgCount, tmpImages);
            defaultImages = new();
            images = new();
            imagesViews = new();
            for (uint i = 0; i < imgCount; ++i)
            {
                defaultImages.Add(tmpImages[i]);
                var image = new VulImage(device, tmpImages[i]);
                images.Add(image);
                ImageViewCreateInfo viewCreateInfo = new();
                viewCreateInfo.SType = StructureType.ImageViewCreateInfo;
                viewCreateInfo.Image = tmpImages[i];
                viewCreateInfo.ViewType = ImageViewType.Type2D;
                viewCreateInfo.Format = format.Format;
                viewCreateInfo.Components.R = ComponentSwizzle.Identity;
                viewCreateInfo.Components.G = ComponentSwizzle.Identity;
                viewCreateInfo.Components.B = ComponentSwizzle.Identity;
                viewCreateInfo.Components.A = ComponentSwizzle.Identity;
                viewCreateInfo.SubresourceRange.AspectMask =
                    ImageAspectFlags.ColorBit;
                viewCreateInfo.SubresourceRange.BaseMipLevel = 0;
                viewCreateInfo.SubresourceRange.LevelCount = 1;
                viewCreateInfo.SubresourceRange.BaseArrayLayer = 0;
                viewCreateInfo.SubresourceRange.LayerCount = 1;
                imagesViews.Add(images[(int)i].createImageView(viewCreateInfo));
            }
        }
    }

    private unsafe void gatherSwapChainSupportDetails(out SwapChainSupportDetails output)
    {
        output = new();
        output.formats = new();
        output.presentModes = new();
        SurfaceCapabilitiesKHR cpbs = new();
        VulResultException.checkResult("Failed to gather surface capabilities: ",
            instance.getSwapchainExtension()
                .GetPhysicalDeviceSurfaceCapabilities(device.getBaseDevice().getBase(), surface, &cpbs));
        output.capabilities = cpbs;
        uint formatCount;
        VulResultException.checkResult("Failed to gather surface formats: ", instance.getSwapchainExtension()
            .GetPhysicalDeviceSurfaceFormats(device.getBaseDevice().getBase(), surface, &formatCount, null));
        SurfaceFormatKHR[] formats = new SurfaceFormatKHR[(int)formatCount];
        VulResultException.checkResult("Failed to gather surface formats: ", instance.getSwapchainExtension()
            .GetPhysicalDeviceSurfaceFormats(device.getBaseDevice().getBase(), surface, &formatCount, formats));
        foreach (var surfaceFormatKhr in formats)
        {
            output.formats.Add(surfaceFormatKhr);
        }

        VulResultException.checkResult("Failed to gather surface present modes: ", instance.getSwapchainExtension()
            .GetPhysicalDeviceSurfacePresentModes(device.getBaseDevice().getBase(), surface, &formatCount, null));

        PresentModeKHR[] presentModes = new PresentModeKHR[formatCount];
        VulResultException.checkResult("Failed to gather surface present modes: ", instance.getSwapchainExtension()
            .GetPhysicalDeviceSurfacePresentModes(device.getBaseDevice().getBase(), surface, &formatCount,
                presentModes));
        foreach (var presentModeKhr in presentModes)
        {
            output.presentModes.Add(presentModeKhr);
        }
    }

    private SurfaceFormatKHR
        chooseSurfaceFormat(List<SurfaceFormatKHR> formats)
    {
        List<Format> preferredFormats = new List<Format>()
        {
            Format.R64G64B64A64Sfloat, Format.R32G32B32A32Sfloat,
            Format.R16G16B16A16Sfloat, Format.B8G8R8A8Srgb
        };
        SurfaceFormatKHR selectedFormat = new();
        selectedFormat.Format = (0);
        selectedFormat.ColorSpace = ColorSpaceKHR.SpaceSrgbNonlinearKhr;
        foreach (var item in formats)
        {
            foreach (var citem in preferredFormats)
            {
                if (item.Format == citem &&
                    (item.Format > selectedFormat.Format ||
                     (item.ColorSpace >= selectedFormat.ColorSpace &&
                      item.Format >= selectedFormat.Format)))
                {
                    selectedFormat = item;
                }
            }
        }

        if (selectedFormat.Format != 0)
        {
            return selectedFormat;
        }

        return formats[0];
    }

    private PresentModeKHR
        choosePresentMode(List<PresentModeKHR> presentModes,
            bool enableFrameLock)
    {
        foreach (var presentMode in presentModes)
        {
            if (presentMode == (enableFrameLock
                    ? PresentModeKHR.MailboxKhr
                    : PresentModeKHR.ImmediateKhr))
            {
                return presentMode;
            }
        }

        return PresentModeKHR.FifoKhr;
    }

    private Extent2D chooseSwapchainExtent(uint width, uint height,
        SurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.CurrentExtent.Width != uint.MaxValue)
        {
            return capabilities.CurrentExtent;
        }
        else
        {
            Extent2D extent = new(width, height);

            extent.Width =
                uint.Min(capabilities.MaxImageExtent.Width,
                    uint.Max(capabilities.MinImageExtent.Width, extent.Width));

            extent.Height =
                uint.Min(capabilities.MaxImageExtent.Height,
                    uint.Max(capabilities.MinImageExtent.Height, extent.Height));

            return extent;
        }
    }

    private void cleanUpImages() {
        foreach (var vulImageView in imagesViews)
        {
            vulImageView.destroy();
        }
        
        imagesViews.Clear();
        images.Clear();
    }
    
    public override unsafe void destroy()
    {
        cleanUpImages();
        swapchainExtension.DestroySwapchain(device.getDevice(), swapchain, null);
        destroyed = true;
    }
}