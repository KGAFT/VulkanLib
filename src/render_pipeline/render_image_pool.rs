use crate::device::image::image::VlImage;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;


//@TODO gain full control to pipeline of image!!!
pub struct RenderImagePool {
    color_images: Vec<(VlImage, bool)>,
    depth_images: Vec<(VlImage, bool)>,
    cube_images: Vec<(VlImage, bool)>,
}

impl RenderImagePool {
    pub fn new() -> Self {
        Self {
            color_images: Vec::new(),
            depth_images: Vec::new(),
            cube_images: Vec::new(),
        }
    }
    
    pub fn cleanup_all(&mut self) {
        self.color_images.clear();
        self.depth_images.clear();
        self.cube_images.clear();
    }

    pub fn acquire_depth_image(&mut self, device: &VlLogicalDevice, extent: (u32, u32)) -> VlImage {
        for x in self.depth_images.iter_mut() {
            if !x.1 {
                x.0.resize(device, extent.0, extent.1);
                x.1 = true;
                return x.0.clone();
            }
        }
        let image = Self::create_depth_attachment(device, extent);
        self.depth_images.push((image, true));
        self.cleanup_depth();
        return self.depth_images.last().unwrap().0.clone();
    }

    pub fn acquire_color_image(&mut self, device: &VlLogicalDevice, extent: (u32, u32)) -> VlImage {
        for x in self.color_images.iter_mut() {
            if !x.1 {
                x.0.resize(device, extent.0, extent.1);
                x.1 = true;
                return x.0.clone();
            }
        }
        let image = Self::create_color_attachment(device, extent);
        self.color_images.push((image, true));
        self.cleanup_color();
        return self.color_images.last().unwrap().0.clone();
    }

    pub fn acquire_cube_image(&mut self, device: &VlLogicalDevice, extent: (u32, u32)) -> VlImage {
        for x in self.cube_images.iter_mut() {
            if !x.1 {
                x.0.resize(device, extent.0, extent.1);
                x.1 = true;
                return x.0.clone();
            }
        }
        let image = Self::create_cube_attachment(device, extent);
        self.cube_images.push((image, true));
        self.cleanup_cube();
        return self.cube_images.last().unwrap().0.clone();
    }

    pub fn release_cube_image(&mut self, image: &VlImage) {
        for x in self.cube_images.iter_mut() {
            if x.0.image() == image.image() {
                x.1 = false;
                break;
            }
        }
    }

    pub fn release_color_image(&mut self, image: &VlImage) {
        for x in self.color_images.iter_mut() {
            if x.0.image() == image.image() {
                x.1 = false;
                break;
            }
        }
    }

    pub fn release_depth_image(&mut self, image: &VlImage) {
        for x in self.depth_images.iter_mut() {
            if x.0.image() == image.image() {
                x.1 = false;
                break;
            }
        }
    }

    pub fn cleanup_depth(&mut self) {
        let mut indices_to_remove = Vec::new();
        for i in 0..self.depth_images.len() {
            if !self.depth_images[i].1 {
                indices_to_remove.push(i);
            }
        }
        while indices_to_remove.len() > 1 {
            let index = indices_to_remove.pop().unwrap();
            let obj = self.depth_images.remove(index);
            drop(obj.0);
        }
    }

    pub fn cleanup_color(&mut self) {
        let mut indices_to_remove = Vec::new();
        for i in 0..self.color_images.len() {
            if !self.color_images[i].1 {
                indices_to_remove.push(i);
            }
        }
        while indices_to_remove.len() > 1 {
            let index = indices_to_remove.pop().unwrap();
            let obj = self.color_images.remove(index);
            drop(obj.0);
        }
    }

    pub fn cleanup_cube(&mut self) {
        let mut indices_to_remove = Vec::new();
        for i in 0..self.cube_images.len() {
            if !self.cube_images[i].1 {
                indices_to_remove.push(i);
            }
        }
        while indices_to_remove.len() > 1 {
            let index = indices_to_remove.pop().unwrap();
            let obj = self.cube_images.remove(index);
            drop(obj.0);
        }
    }

    fn create_depth_attachment(device: &VlLogicalDevice, extent: (u32, u32)) -> VlImage {
        let format = device
            .find_depth_format()
            .expect("Failed to find depth format");
        let mut create_info = default_depth_image_create_info(extent.0, extent.1);
        create_info = create_info.format(format);
        let mut image = VlImage::new(device, create_info);
        let view_info = default_depth_view_create_info(image.image());
        image.create_image_view(view_info);
        image.transition_image_layout_q(
            device
                .find_queue_by_type_r(vk::QueueFlags::GRAPHICS)
                .unwrap(),
            vk::ImageLayout::UNDEFINED,
            vk::ImageLayout::GENERAL,
            vk::ImageAspectFlags::DEPTH,
        );
        image
    }

    fn create_color_attachment(device: &VlLogicalDevice, extent: (u32, u32)) -> VlImage {
        let create_info = default_color_image_create_info(extent.0, extent.1);
        let mut image = VlImage::new(device, create_info);
        let view_info = default_color_view_create_info(image.image());
        image.create_image_view(view_info);
        image.transition_image_layout_q(
            device
                .find_queue_by_type_r(vk::QueueFlags::GRAPHICS)
                .unwrap(),
            vk::ImageLayout::UNDEFINED,
            vk::ImageLayout::GENERAL,
            vk::ImageAspectFlags::COLOR,
        );
        image
    }

    fn create_cube_attachment(device: &VlLogicalDevice, extent: (u32, u32)) -> VlImage {
        let create_info = default_cube_color_image_create_info(extent.0, extent.1);
        let mut image = VlImage::new(device, create_info);
        let cb_view_info = default_cube_view_create_info(image.image());
        let mut view_info = default_color_view_create_info(image.image());
        image.create_image_view(cb_view_info);
        for i in 0..6 {
            view_info.image = image.image();
            view_info.subresource_range.base_array_layer = i;
            image.create_image_view(view_info.clone());
        }
        return image;
    }
}

/// -----------------------
/// Color 2D Image
/// -----------------------
pub fn default_color_image_create_info(width: u32, height: u32) -> vk::ImageCreateInfo<'static> {
    vk::ImageCreateInfo::default()
        .flags(vk::ImageCreateFlags::empty())
        .image_type(vk::ImageType::TYPE_2D)
        .format(vk::Format::R32G32B32A32_SFLOAT)
        .extent(vk::Extent3D {
            width,
            height,
            depth: 1,
        })
        .mip_levels(1)
        .array_layers(1)
        .samples(vk::SampleCountFlags::TYPE_1)
        .tiling(vk::ImageTiling::OPTIMAL)
        .usage(
            vk::ImageUsageFlags::COLOR_ATTACHMENT
                | vk::ImageUsageFlags::SAMPLED
                | vk::ImageUsageFlags::TRANSFER_SRC
                | vk::ImageUsageFlags::STORAGE,
        )
        .sharing_mode(vk::SharingMode::EXCLUSIVE)
        .initial_layout(vk::ImageLayout::UNDEFINED)
}

/// -----------------------
/// Color Cube Image (6 layers)
/// -----------------------
pub fn default_cube_color_image_create_info(
    width: u32,
    height: u32,
) -> vk::ImageCreateInfo<'static> {
    vk::ImageCreateInfo::default()
        .flags(vk::ImageCreateFlags::CUBE_COMPATIBLE)
        .image_type(vk::ImageType::TYPE_2D)
        .format(vk::Format::R32G32B32A32_SFLOAT)
        .extent(vk::Extent3D {
            width,
            height,
            depth: 1,
        })
        .mip_levels(1)
        .array_layers(6)
        .samples(vk::SampleCountFlags::TYPE_1)
        .tiling(vk::ImageTiling::OPTIMAL)
        .usage(
            vk::ImageUsageFlags::COLOR_ATTACHMENT
                | vk::ImageUsageFlags::SAMPLED
                | vk::ImageUsageFlags::TRANSFER_SRC
                | vk::ImageUsageFlags::STORAGE,
        )
        .sharing_mode(vk::SharingMode::EXCLUSIVE)
        .initial_layout(vk::ImageLayout::UNDEFINED)
}

/// -----------------------
/// 2D Color Image View
/// -----------------------
pub fn default_color_view_create_info(image: vk::Image) -> vk::ImageViewCreateInfo<'static> {
    vk::ImageViewCreateInfo::default()
        .image(image)
        .view_type(vk::ImageViewType::TYPE_2D)
        .format(vk::Format::R32G32B32A32_SFLOAT)
        .subresource_range(vk::ImageSubresourceRange {
            aspect_mask: vk::ImageAspectFlags::COLOR,
            base_mip_level: 0,
            level_count: 1,
            base_array_layer: 0,
            layer_count: 1,
        })
}

/// -----------------------
/// Cube Color Image View
/// -----------------------
pub fn default_cube_view_create_info(image: vk::Image) -> vk::ImageViewCreateInfo<'static> {
    vk::ImageViewCreateInfo::default()
        .image(image)
        .view_type(vk::ImageViewType::CUBE)
        .format(vk::Format::R32G32B32A32_SFLOAT)
        .subresource_range(vk::ImageSubresourceRange {
            aspect_mask: vk::ImageAspectFlags::COLOR,
            base_mip_level: 0,
            level_count: 1,
            base_array_layer: 0,
            layer_count: 6,
        })
}

/// -----------------------
/// Depth Image
/// -----------------------
pub fn default_depth_image_create_info(width: u32, height: u32) -> vk::ImageCreateInfo<'static> {
    vk::ImageCreateInfo::default()
        .flags(vk::ImageCreateFlags::empty())
        .image_type(vk::ImageType::TYPE_2D)
        .format(vk::Format::D32_SFLOAT)
        .extent(vk::Extent3D {
            width,
            height,
            depth: 1,
        })
        .mip_levels(1)
        .array_layers(1)
        .samples(vk::SampleCountFlags::TYPE_1)
        .tiling(vk::ImageTiling::OPTIMAL)
        .usage(
            vk::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT
                | vk::ImageUsageFlags::SAMPLED
                | vk::ImageUsageFlags::TRANSFER_SRC,
        )
        .sharing_mode(vk::SharingMode::EXCLUSIVE)
        .initial_layout(vk::ImageLayout::UNDEFINED)
}

/// -----------------------
/// Depth Image View
/// -----------------------
pub fn default_depth_view_create_info(image: vk::Image) -> vk::ImageViewCreateInfo<'static> {
    vk::ImageViewCreateInfo::default()
        .image(image)
        .view_type(vk::ImageViewType::TYPE_2D)
        .format(vk::Format::D32_SFLOAT)
        .subresource_range(vk::ImageSubresourceRange {
            aspect_mask: vk::ImageAspectFlags::DEPTH,
            base_mip_level: 0,
            level_count: 1,
            base_array_layer: 0,
            layer_count: 1,
        })
}
