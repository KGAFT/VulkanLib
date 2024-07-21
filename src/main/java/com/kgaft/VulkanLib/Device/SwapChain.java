package com.kgaft.VulkanLib.Device;

import java.lang.instrument.IllegalClassFormatException;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;

import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import org.lwjgl.vulkan.*;

import com.kgaft.VulkanLib.Device.Image.Image;
import com.kgaft.VulkanLib.Device.Image.ImageView;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;

import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.KHRSurface.*;
import static org.lwjgl.vulkan.KHRSwapchain.*;

class SwapChainSupportDetails {
    public LwjglObject<VkSurfaceCapabilitiesKHR> capabilities = new LwjglObject<>(VkSurfaceCapabilitiesKHR.class);
    public LwjglObject<VkSurfaceFormatKHR.Buffer> formats;
    public int[] presentModes;

    SwapChainSupportDetails() throws IllegalClassFormatException {
    }
}

public class SwapChain extends DestroyableObject {
    private static LwjglObject<VkSurfaceFormatKHR.Buffer> formatsBuffer;
    private static VkSurfaceFormatKHR format;

    public static VkSurfaceFormatKHR getFormat() {
        return format;
    }

    private LogicalDevice device;
    private long swapchainKhr;
    private long surface;
    private int presentMode;
    private LwjglObject<VkExtent2D> extent;
    private long[] baseImages;
    private List<Image> swapchainImages = new ArrayList<>();
    private List<ImageView> swapchainImageViews = new ArrayList<>();
    private int width;
    private int height;
    private boolean enableFrameLock = false;

    public SwapChain(LogicalDevice device, long surface,
                     int width, int height, boolean enableFrameLock) throws IllegalClassFormatException, VkErrorException {
        this.device = device;
        this.surface = surface;
        this.width = width;
        this.height = height;
        this.enableFrameLock = enableFrameLock;
        createSwapChain(width, height, enableFrameLock);
    }

    public void recreate(int width, int height) throws IllegalClassFormatException, VkErrorException {
        destroy();
        destroyed = false;
        this.width = width;
        this.height = height;

        createSwapChain(width, height, enableFrameLock);
    }

    public void recreate(int width, int height, boolean refreshRateLock) throws IllegalClassFormatException, VkErrorException {
        destroy();
        destroyed = false;
        enableFrameLock = refreshRateLock;
        this.width = width;
        this.height = height;
        createSwapChain(width, height, refreshRateLock);
    }

    public List<Image> getSwapchainImages() {
        return swapchainImages;
    }

    public List<ImageView> getSwapchainImageViews() {
        return swapchainImageViews;
    }

    private void createSwapChain(int width, int height, boolean enableFrameLock) throws IllegalClassFormatException, VkErrorException {
        SwapChainSupportDetails support = new SwapChainSupportDetails();

        gatherSwapChainSupportDetails(support);
        formatsBuffer = support.formats;
        format = chooseSurfaceFormat(support.formats);
        presentMode = choosePresentMode(support.presentModes, enableFrameLock);
        extent = chooseSwapchainExtent(width, height, support.capabilities);
        int imageCount = support.capabilities.get().minImageCount() + 1;
        if (support.capabilities.get().maxImageCount() > 0 &&
                imageCount > support.capabilities.get().maxImageCount()) {
            imageCount = support.capabilities.get().maxImageCount();
        }
        LwjglObject<VkSwapchainCreateInfoKHR> createInfo = new LwjglObject<>(VkSwapchainCreateInfoKHR.class);
        createInfo.get().surface(surface);
        createInfo.get().minImageCount(imageCount);
        createInfo.get().imageFormat(format.format());
        createInfo.get().imageColorSpace(format.colorSpace());
        createInfo.get().imageExtent(extent.get());
        createInfo.get().imageArrayLayers(1);
        createInfo.get().imageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        HashSet<Integer> baseQIndices = new HashSet<Integer>();
        baseQIndices.add(device.getQueueByType(VK_QUEUE_GRAPHICS_BIT).getIndex());
        baseQIndices.add(device.getPresentQueue().getIndex());


        IntBuffer queueIndices = IntBuffer.allocate(baseQIndices.size());
        baseQIndices.forEach(queueIndices::put);
        queueIndices.rewind();
        if (queueIndices.capacity() > 1) {
            createInfo.get().imageSharingMode(VK_SHARING_MODE_CONCURRENT);
            createInfo.get().pQueueFamilyIndices(queueIndices);
        } else {
            createInfo.get().imageSharingMode(VK_SHARING_MODE_EXCLUSIVE);
        }
        createInfo.get().preTransform(support.capabilities.get().currentTransform());
        createInfo.get().compositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        createInfo.get().presentMode(presentMode);
        createInfo.get().clipped(true);
        long[] preSwap = new long[1];
        VkErrorException.checkVkStatus("Failed to create swapchain", vkCreateSwapchainKHR(device.getDevice(), createInfo.get(), null, preSwap));
        this.swapchainKhr = preSwap[0];
        int[] swapImagesC = new int[1];
        vkGetSwapchainImagesKHR(device.getDevice(), swapchainKhr, swapImagesC, null);
        baseImages = new long[swapImagesC[0]];
        vkGetSwapchainImagesKHR(device.getDevice(), swapchainKhr, swapImagesC, baseImages);

        Arrays.stream(baseImages).forEach(element -> {
            try {
                Image img = new Image(device, element);
                swapchainImages.add(img);
                LwjglObject<VkImageViewCreateInfo> viewCreateInfo = new LwjglObject<>(VkImageViewCreateInfo.class);
                viewCreateInfo.get().image(element);
                viewCreateInfo.get().viewType(VK_IMAGE_VIEW_TYPE_2D);
                viewCreateInfo.get().format(format.format());
                viewCreateInfo.get().components().r(VK_COMPONENT_SWIZZLE_IDENTITY);
                viewCreateInfo.get().components().g(VK_COMPONENT_SWIZZLE_IDENTITY);
                viewCreateInfo.get().components().b(VK_COMPONENT_SWIZZLE_IDENTITY);
                viewCreateInfo.get().components().a(VK_COMPONENT_SWIZZLE_IDENTITY);
                viewCreateInfo.get().subresourceRange().aspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
                viewCreateInfo.get().subresourceRange().baseMipLevel(0);
                viewCreateInfo.get().subresourceRange().levelCount(1);
                viewCreateInfo.get().subresourceRange().baseArrayLayer(0);
                viewCreateInfo.get().subresourceRange().layerCount(1);
                swapchainImageViews.add(img.createImageView(viewCreateInfo));

            } catch (IllegalClassFormatException e) {
                throw new RuntimeException(e);
            } catch (VkErrorException e) {
                throw new RuntimeException(e);
            }

        });

    }

    private void gatherSwapChainSupportDetails(SwapChainSupportDetails output) throws VkErrorException, IllegalClassFormatException {
        int res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getBase().getBase(), surface, output.capabilities.get());

        VkErrorException.checkVkStatus("Failed to gather surface capabilities", res);
        int[] formatCount = new int[1];
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(device.getBase().getBase(), surface, formatCount, null);
        VkErrorException.checkVkStatus("Failed to gather surface formats", res);
        output.formats = new LwjglObject<>(VkSurfaceFormatKHR.class, VkSurfaceFormatKHR.Buffer.class, formatCount[0]);
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(device.getBase().getBase(), surface, formatCount, output.formats.get());
        VkErrorException.checkVkStatus("Failed to gather surface formats", res);
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(device.getBase().getBase(), surface, formatCount, null);
        VkErrorException.checkVkStatus("Failed to gather surface present modes", res);
        output.presentModes = new int[formatCount[0]];
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(device.getBase().getBase(), surface, formatCount, output.presentModes);
        VkErrorException.checkVkStatus("Failed to gather surface present modes", res);
    }

    private VkSurfaceFormatKHR
    chooseSurfaceFormat(LwjglObject<VkSurfaceFormatKHR.Buffer> formats) {
        int[] preferredFormats = new int[]{VK_FORMAT_R64G64B64A64_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SNORM,
                VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_USCALED, VK_FORMAT_R16G16B16A16_SSCALED, VK_FORMAT_B16G16R16G16_422_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB};
        VkSurfaceFormatKHR selectedFormat = null;
        for (VkSurfaceFormatKHR item : formats.get()) {
            for (int citem : preferredFormats) {
                if (item.format() == citem) {
                    if (selectedFormat != null) {
                        if (item.format() > selectedFormat.format() || (item.colorSpace() >= selectedFormat.colorSpace() && item.format() >= selectedFormat.format())) {
                            selectedFormat = item;
                        }
                    } else {
                        selectedFormat = item;
                    }

                }
            }
        }
        if (selectedFormat != null) {
            return selectedFormat;
        }
        formats.get().rewind();
        return formats.get().get();
    }

    private int
    choosePresentMode(int[] presentModes, boolean enableFrameLock) {
        for (int presentMode : presentModes) {
            if (presentMode == (enableFrameLock ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)) {
                return presentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    private LwjglObject<VkExtent2D> chooseSwapchainExtent(int width, int height,
                                                          LwjglObject<VkSurfaceCapabilitiesKHR> capabilities) throws IllegalClassFormatException {

        if (capabilities.get().currentExtent().width() != Integer.MAX_VALUE) {
            return new LwjglObject<>(capabilities.get().currentExtent(), VkExtent2D.class);
        } else {
            LwjglObject<VkExtent2D> res = new LwjglObject<>(VkExtent2D.class);
            res.get().width(width);
            res.get().height(height);
            extent.get().width(
                    Math.min(capabilities.get().maxImageExtent().width(),
                            Math.max(capabilities.get().minImageExtent().width(), extent.get().width())));

            extent.get().height(
                    Math.min(capabilities.get().maxImageExtent().height(),
                            Math.max(capabilities.get().minImageExtent().height(), extent.get().height())));

            return extent;
        }
    }

    @Override
    public void destroy() {
        destroyed = true;
        swapchainImageViews.forEach(ImageView::destroy);
        swapchainImages.clear();
        vkDestroySwapchainKHR(device.getDevice(), swapchainKhr, null);
    }
}
