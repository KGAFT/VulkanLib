using Silk.NET.Vulkan;
using Silk.NET.Vulkan.Extensions.KHR;
using VulkanLib.Device.Image;
using VulkanLib.Device.LogicalDevice;

namespace VulkanLib.Device;

struct SwapChainSupportDetails {
    public SurfaceCapabilitiesKHR capabilities;
    public List<SurfaceFormatKHR> formats = new();
    public List<PresentModeKHR> presentModes = new();

    public SwapChainSupportDetails()
    {
        capabilities = default;
    }
}

public class VulSwapChain
{
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
    
    private unsafe void createSwapChain(uint width, uint height, bool enableFrameLock) {
    SwapChainSupportDetails support = new();
    gatherSwapChainSupportDetails(support);

    format = chooseSurfaceFormat(support.formats);
    presentMode = choosePresentMode(support.presentModes, enableFrameLock);
    extent = chooseSwapchainExtent(width, height, support.capabilities);
    uint imageCount = support.capabilities.MinImageCount + 1;
    if (support.capabilities.MaxImageCount > 0 &&
        imageCount > support.capabilities.MaxImageCount) {
      imageCount = support.capabilities.MaxImageCount;
    }
    SwapchainCreateInfoKHR createInfo = new(StructureType.SwapchainCreateInfoKhr,
        null, 0, surface, imageCount, format.Format,
        format.ColorSpace, extent, 1, ImageUsageFlags.ColorAttachmentBit);
    HashSet<uint> queueIndices = new HashSet<uint>(){
        device.getQueueByType(QueueFlags.GraphicsBit).getIndex(),
        device.getPresentQueue().getIndex()};
    var tmpindices = new List<uint>(queueIndices.ToArray());
    fixed (uint* pIndices = tmpindices.ToArray())
    {
        if (queueIndices.Count > 1) {
            createInfo.ImageSharingMode = SharingMode.Concurrent;
            createInfo.QueueFamilyIndexCount = (uint)tmpindices.Count;
            createInfo.PQueueFamilyIndices = pIndices;
        } else {
            createInfo.ImageSharingMode = SharingMode.Exclusive;
        }
        createInfo.PreTransform = support.capabilities.CurrentTransform;
        createInfo.CompositeAlpha = CompositeAlphaFlagsKHR.OpaqueBitKhr;
        createInfo.PresentMode = presentMode;
        createInfo.Clipped = true;

        try {
            if (swapchainExtension==null)
            {
               Vk.GetApi().TryGetDeviceExtension(device.getInstance().getBase(), device.getDevice(), out swapchainExtension);
            }
            swapchainKhr = device->getDevice().createSwapchainKHR(createInfo, VkLibAlloc::acquireAllocCb().get());
            baseImages = device->getDevice().getSwapchainImagesKHR(swapchainKhr);
            swapchainImages.resize(baseImages.size());
            for (uint32_t i = 0; i < baseImages.size(); ++i) {
                swapchainImages[i] =
                    std::shared_ptr<Image>(new Image(device, baseImages[i]));
                vk::ImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.image = baseImages[i];
                viewCreateInfo.viewType = vk::ImageViewType::e2D;
                viewCreateInfo.format = format.format;
                viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
                viewCreateInfo.subresourceRange.aspectMask =
                    vk::ImageAspectFlagBits::eColor;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;
                swapchainImageViews.push_back(
                    swapchainImages[i]->createImageView(viewCreateInfo));
            }
        } catch (vk::SystemError &err) {
            std::cerr << err.what() << std::endl;
        }
    }
    
  }

  void gatherSwapChainSupportDetails(SwapChainSupportDetails &output) {
    vk::Result res;
    res = device->getBaseDevice()->getBase().getSurfaceCapabilitiesKHR(
        surface, &output.capabilities);
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    uint32_t formatCount;
    res = device->getBaseDevice()->getBase().getSurfaceFormatsKHR(
        surface, &formatCount, nullptr);
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    output.formats.resize(formatCount);
    res = device->getBaseDevice()->getBase().getSurfaceFormatsKHR(
        surface, &formatCount, output.formats.data());
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    res = device->getBaseDevice()->getBase().getSurfacePresentModesKHR(
        surface, &formatCount, nullptr);
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
    output.presentModes.resize(formatCount);
    res = device->getBaseDevice()->getBase().getSurfacePresentModesKHR(
        surface, &formatCount, output.presentModes.data());
    if (res != vk::Result::eSuccess) {
      std::cerr << "Failed to gather swap chain info" << std::endl;
    }
  }

  vk::SurfaceFormatKHR
  chooseSurfaceFormat(std::vector<vk::SurfaceFormatKHR> &formats) {
    std::vector<vk::Format> preferredFormats = {
        vk::Format::eR64G64B64A64Sfloat, vk::Format::eR32G32B32A32Sfloat,
        vk::Format::eR16G16B16A16Sfloat, vk::Format::eB8G8R8A8Srgb};
    vk::SurfaceFormatKHR selectedFormat{};
    selectedFormat.format = static_cast<vk::Format>(0);
    selectedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    for (const auto &item : formats) {
      for (const auto &citem : preferredFormats) {
        if (item.format == citem &&
            (item.format > selectedFormat.format ||
             (item.colorSpace >= selectedFormat.colorSpace &&
              item.format >= selectedFormat.format))) {
          selectedFormat = item;
        }
      }
    }
    if (static_cast<int>(selectedFormat.format) != 0) {
      return selectedFormat;
    }
    return formats[0];
  }

  vk::PresentModeKHR
  choosePresentMode(std::vector<vk::PresentModeKHR> &presentModes,
                    bool enableFrameLock) {
    for (vk::PresentModeKHR presentMode : presentModes) {
      if (presentMode == (enableFrameLock ? vk::PresentModeKHR::eMailbox
                                          : vk::PresentModeKHR::eImmediate)) {
        return presentMode;
      }
    }

    return vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D chooseSwapchainExtent(uint32_t width, uint32_t height,
                                     vk::SurfaceCapabilitiesKHR &capabilities) {

    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    } else {
      vk::Extent2D extent = {width, height};

      extent.width =
          std::min(capabilities.maxImageExtent.width,
                   std::max(capabilities.minImageExtent.width, extent.width));

      extent.height =
          std::min(capabilities.maxImageExtent.height,
                   std::max(capabilities.minImageExtent.height, extent.height));

      return extent;
    }
  }
}