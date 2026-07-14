use crate::device::image::image::VlImage;
use crate::device::image::image_view::VlImageView;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use crate::instance::instance::VlInstance;
use ash::khr::swapchain;
use ash::vk;
use std::collections::HashSet;

#[derive(Default)]
struct SwapChainDetails {
    capabilities: vk::SurfaceCapabilitiesKHR,
    formats: Vec<vk::SurfaceFormatKHR>,
    present_modes: Vec<vk::PresentModeKHR>,
}
/**
    VlSwapChain is not supported shairing, like other classes,
    if you need to share it between threads, use Arc Mutex,
    images from swapchain not shared as well, despite that other images are shared compatible via clone
*/
pub struct VlSwapChain {
    format: vk::SurfaceFormatKHR,
    present_mode: vk::PresentModeKHR,
    extent: vk::Extent2D,
    images: Vec<VlImage>,
    image_views: Vec<VlImageView>,
    swap_chain_loader: swapchain::Device,
    device: VlLogicalDevice,
    instance: VlInstance,
    swap_chain: vk::SwapchainKHR,
    surface: vk::SurfaceKHR,
    frame_lock: bool
}

impl Drop for VlSwapChain {
    fn drop(&mut self) {
        self.destroy();
    }
}

unsafe impl Send for VlSwapChain {

}

unsafe impl Sync for VlSwapChain {

}

impl VlSwapChain {
    pub fn new(
        device: VlLogicalDevice,
        instance: VlInstance,
        surface: vk::SurfaceKHR,
        width: u32,
        height: u32,
        frame_lock: bool,
    ) -> Self {
        let swap_chain_loader =
            swapchain::Device::new(instance.get_instance_r(), device.device_r());
        let mut pre_res = Self {
            format: Default::default(),
            device,
            instance,
            swap_chain_loader,
            surface,
            present_mode: Default::default(),
            extent: Default::default(),
            images: vec![],
            image_views: vec![],
            swap_chain: Default::default(),
            frame_lock
        };
        pre_res.create_swap_chain(width, height, frame_lock, surface);
        pre_res
    }

    fn create_swap_chain(
        &mut self,
        width: u32,
        height: u32,
        frame_lock: bool,
        surface: vk::SurfaceKHR,
    ) {
        let mut support = SwapChainDetails::default();
        Self::gather_swap_chain_info(&self.instance, &self.device, surface, &mut support);
        self.format = Self::choose_surface_format(support.formats.as_slice());
        self.present_mode = Self::choose_present_mode(support.present_modes.as_slice(), frame_lock);
        self.extent = Self::choose_swapchain_extent(width, height, &support.capabilities);
        let mut image_count = support.capabilities.min_image_count + 1;
        if support.capabilities.max_image_count > 0
            && image_count > support.capabilities.max_image_count
        {
            image_count = support.capabilities.max_image_count;
        }

        let mut queue_indices: Vec<u32> = vec![
            self.device
                .find_queue_by_type_r(vk::QueueFlags::GRAPHICS)
                .unwrap()
                .index(),
            self.device.find_present_queue_r().unwrap().index(),
        ];
        let unique_indices: HashSet<u32> = queue_indices.drain(..).collect();
        let mut queue_indices: Vec<u32> = unique_indices.into_iter().collect();

        // Ensure stable ordering (important for safety across drivers)
        queue_indices.sort_unstable();

        // Default create info
        let mut create_info = vk::SwapchainCreateInfoKHR::default()
            .surface(surface)
            .min_image_count(image_count)
            .image_format(self.format.format)
            .image_color_space(self.format.color_space)
            .image_extent(self.extent)
            .image_array_layers(1)
            .image_usage(vk::ImageUsageFlags::COLOR_ATTACHMENT)
            .pre_transform(support.capabilities.current_transform)
            .composite_alpha(vk::CompositeAlphaFlagsKHR::OPAQUE)
            .present_mode(self.present_mode)
            .clipped(true)
            .old_swapchain(vk::SwapchainKHR::null());

        if queue_indices.len() > 1 {
            create_info = create_info
                .image_sharing_mode(vk::SharingMode::CONCURRENT)
                .queue_family_indices(&queue_indices);
        } else {
            create_info = create_info.image_sharing_mode(vk::SharingMode::EXCLUSIVE);
        }
        self.swap_chain = unsafe { self.swap_chain_loader.create_swapchain(&create_info, None) }
            .expect("Failed to create swap chain");
        let mut images = unsafe {
            self.swap_chain_loader
                .get_swapchain_images(self.swap_chain.clone())
                .expect("Failed to get swap chain images")
        };

        while let Some(image) = images.pop() {
            let mut image = VlImage::from_image(self.device.device(), image);
            let view_create_info = vk::ImageViewCreateInfo {
                image: image.image(),
                view_type: vk::ImageViewType::TYPE_2D,
                format: self.format.format,
                components: vk::ComponentMapping {
                    r: vk::ComponentSwizzle::IDENTITY,
                    g: vk::ComponentSwizzle::IDENTITY,
                    b: vk::ComponentSwizzle::IDENTITY,
                    a: vk::ComponentSwizzle::IDENTITY,
                },
                subresource_range: vk::ImageSubresourceRange {
                    aspect_mask: vk::ImageAspectFlags::COLOR,
                    base_mip_level: 0,
                    level_count: 1,
                    base_array_layer: 0,
                    layer_count: 1,
                },
                ..Default::default()
            };
            let view = image.create_image_view(view_create_info);
            self.images.push(image);
            self.image_views.push(view);
        }
    }

    pub fn recreate_swap_chain(&mut self, width: u32, height: u32){
        self.destroy();
        self.create_swap_chain(width, height, self.frame_lock, self.surface);
    }
    
    



    pub fn recreate_swap_chain_framelock(&mut self, width: u32, height: u32, frame_lock: bool){
        self.frame_lock = frame_lock;
        self.destroy();
        self.create_swap_chain(width, height, self.frame_lock, self.surface);
    }



    fn cleanup_images(&mut self) {
        while let Some(view) = self.image_views.pop() {
            drop(view)
        }
        self.image_views.clear();
        self.images.clear();
    }

    fn destroy(&mut self) {
        self.cleanup_images();
        unsafe {
            self.device.device().device_wait_idle().unwrap();
            self.swap_chain_loader
                .destroy_swapchain(self.swap_chain, None);
        }
    }

    fn gather_swap_chain_info(
        instance: &VlInstance,
        device: &VlLogicalDevice,
        surface: vk::SurfaceKHR,
        res: &mut SwapChainDetails,
    ) {
        let capab = unsafe {
            instance
                .surface_loader()
                .expect("Surface was not loaded")
                .get_physical_device_surface_capabilities(device.base_device().inner(), surface)
        }
        .expect("Failed to get surface capabilities");
        let formats = unsafe {
            instance
                .surface_loader()
                .unwrap()
                .get_physical_device_surface_formats(device.base_device().inner(), surface)
        }
        .expect("Failed to get surface formats");
        let present_modes = unsafe {
            instance
                .surface_loader()
                .unwrap()
                .get_physical_device_surface_present_modes(device.base_device().inner(), surface)
                .expect("Failed to get present modes")
        };
        res.present_modes = present_modes;
        res.capabilities = capab;
        res.formats = formats;
    }



    fn choose_surface_format(formats: &[vk::SurfaceFormatKHR]) -> vk::SurfaceFormatKHR {
        // Expanded preferred formats list (HDR → High → Basic)
        let preferred_formats = [
            // High precision float formats
            vk::Format::R64G64B64A64_SFLOAT,
            vk::Format::R32G32B32A32_SFLOAT,
            vk::Format::R32G32B32A32_SFLOAT,
            // HDR10 / HDR formats
            vk::Format::A2B10G10R10_UNORM_PACK32,
            vk::Format::A2R10G10B10_UNORM_PACK32,
            vk::Format::R16G16B16A16_SFLOAT,
            vk::Format::R16G16B16A16_UNORM,
            vk::Format::R16G16B16A16_SNORM,
            // sRGB formats (most common for monitors)
            vk::Format::B8G8R8A8_SRGB,
            vk::Format::R8G8B8A8_SRGB,
            // Basic UNORM/linear
            vk::Format::B8G8R8A8_UNORM,
            vk::Format::R8G8B8A8_UNORM,
        ];

        let mut selected_format = vk::SurfaceFormatKHR {
            format: vk::Format::UNDEFINED,
            color_space: vk::ColorSpaceKHR::SRGB_NONLINEAR,
        };

        for item in formats {
            if preferred_formats.contains(&item.format) {
                if item.format > selected_format.format
                    || (item.color_space >= selected_format.color_space
                        && item.format >= selected_format.format)
                {
                    selected_format = *item;
                }
            }
        }

        if selected_format.format != vk::Format::UNDEFINED {
            selected_format
        } else {
            // fallback: just pick the first format (guaranteed to exist)
            formats[0]
        }
    }

    /// Choose the present mode depending on frame lock.
    /// Mailbox → Immediate → Fifo fallback
    fn choose_present_mode(
        present_modes: &[vk::PresentModeKHR],
        enable_frame_lock: bool,
    ) -> vk::PresentModeKHR {
        let preferred = if enable_frame_lock {
            vk::PresentModeKHR::MAILBOX
        } else {
            vk::PresentModeKHR::IMMEDIATE
        };

        present_modes
            .iter()
            .copied()
            .find(|&m| m == preferred)
            .unwrap_or(vk::PresentModeKHR::FIFO) // guaranteed available
    }

    /// Choose the swapchain extent based on surface capabilities and desired size.
    /// Clamps to min/max range when current_extent is undefined.
    fn choose_swapchain_extent(
        width: u32,
        height: u32,
        capabilities: &vk::SurfaceCapabilitiesKHR,
    ) -> vk::Extent2D {
        if capabilities.current_extent.width != u32::MAX {
            capabilities.current_extent
        } else {
            let clamped_width = capabilities
                .min_image_extent
                .width
                .max(width)
                .min(capabilities.max_image_extent.width);

            let clamped_height = capabilities
                .min_image_extent
                .height
                .max(height)
                .min(capabilities.max_image_extent.height);

            vk::Extent2D {
                width: clamped_width,
                height: clamped_height,
            }
        }
    }

    pub fn swap_chain_loader(&self) -> &swapchain::Device {
        &self.swap_chain_loader
    }

    pub fn swap_chain(&self) -> vk::SwapchainKHR {
        self.swap_chain
    }

    pub fn format(&self) -> vk::SurfaceFormatKHR {
        self.format
    }

    pub fn images(&self) -> &Vec<VlImage> {
        &self.images
    }

    pub fn image_views(&self) -> &Vec<VlImageView> {
        &self.image_views
    }

    pub fn extent(&self) -> vk::Extent2D {
        self.extent
    }
}
