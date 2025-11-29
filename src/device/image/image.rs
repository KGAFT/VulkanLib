use crate::device::image::image_info_owned::ImageCreateInfoOwned;
use crate::device::image::image_view::VlImageView;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use crate::device::logical_device::logical_queue::VlLogicalQueue;
use std::ptr::null_mut;

use ash::vk;
pub struct VlImage {
    base: *mut vk::Image,
    image_info: ImageCreateInfoOwned,
    image_views: Vec<VlImageView>,
    image_memory: *mut vk::DeviceMemory,
    device: ash::Device,
    origin: bool,
}

impl Clone for VlImage {
    fn clone(&self) -> Self {
        Self {
            base: self.base,
            image_info: self.image_info.clone(),
            image_views: self.image_views.clone(),
            image_memory: self.image_memory,
            device: self.device.clone(),
            origin: false,
        }
    }
}

impl Drop for VlImage {
    fn drop(&mut self) {
        if self.origin {
            while let Some(view) = self.image_views.pop() {
                drop(view);
            }
            unsafe {
                self.device.destroy_image(*self.base, None);
                self.device.free_memory(*self.image_memory, None);
            }
        }
    }
}

impl VlImage {
    pub fn new(device: &VlLogicalDevice, create_info: vk::ImageCreateInfo) -> Self {
        let mut res = Self {
            base: null_mut(),
            image_info: ImageCreateInfoOwned::from_base(create_info),
            image_views: vec![],
            image_memory: null_mut(),
            device: device.device(),
            origin: true,
        };
        res.initialize(device);
        res
    }

    pub fn from_image(device: ash::Device, base: vk::Image) -> Self {
        Self {
            base: Box::into_raw(Box::from(base)),
            image_info: ImageCreateInfoOwned {
                ..Default::default()
            },
            image_views: vec![],
            image_memory: null_mut(),
            device,
            origin: false,
        }
    }

    pub fn get_image_views(&self) -> &Vec<VlImageView> {
        &self.image_views
    }

    pub fn get_image_views_clone(&self) -> Vec<VlImageView> {
        self.image_views.clone()
    }

    pub fn image(&self) -> vk::Image {
        unsafe { (*self.base).clone() }
    }

    fn initialize(&mut self, device: &VlLogicalDevice) {
        let temp_info = self.image_info.to_base();
        let image =
            unsafe { self.device.create_image(&temp_info, None) }.expect("Failed to create image");
        let reqs = unsafe { self.device.get_image_memory_requirements(image.clone()) };
        let alloc_info = vk::MemoryAllocateInfo {
            allocation_size: reqs.size,
            memory_type_index: device
                .find_memory_type(reqs.memory_type_bits, vk::MemoryPropertyFlags::DEVICE_LOCAL)
                .expect("failed to find memory type index"),
            ..Default::default()
        };
        let memory = unsafe { self.device.allocate_memory(&alloc_info, None) }
            .expect("Failed to allocate memory");
        unsafe {
            self.device
                .bind_image_memory(image.clone(), memory.clone(), 0)
                .expect("Failed to bind image memory!");
        }
        if self.base == null_mut() {
            self.base = Box::into_raw(Box::from(image));
            self.image_memory = Box::into_raw(Box::from(memory));
        } else {
            unsafe {
                *self.base = image;
                *self.image_memory = memory;
            }
        }
    }

    pub fn create_image_view(&mut self, create_info: vk::ImageViewCreateInfo) -> VlImageView {
        let view = unsafe {
            self.device
                .create_image_view(&create_info, None)
                .expect("Failed to create image view!")
        };
        let view = VlImageView::new(
            self.image_info.extent.clone(),
            self.device.clone(),
            view,
            &create_info,
        );
        let res = view.clone();
        self.image_views.push(view);
        res
    }
    
    

    #[allow(unused_assignments)]
    pub fn resize(&mut self, device: &VlLogicalDevice, width: u32, height: u32) {
        if self.origin {
            self.destroy();
            self.image_info.extent = vk::Extent3D {
                width,
                height,
                depth: 1,
            };
            if self.image_info.initial_layout != vk::ImageLayout::UNDEFINED {
                let temp_layout = self.image_info.initial_layout;
                self.image_info.initial_layout = vk::ImageLayout::UNDEFINED;
                self.initialize(device);
                self.transition_image_layout_q(
                    device
                        .find_queue_by_type_r(vk::QueueFlags::GRAPHICS)
                        .unwrap(),
                    vk::ImageLayout::UNDEFINED,
                    temp_layout,
                    Self::aspect_flags_from_format(self.image_info.format),
                );
                self.image_info.initial_layout = temp_layout;
            } else {
                self.initialize(device);
            }
            unsafe {
                let mut temp_view = vk::ImageViewCreateInfo::default();
                for v in self.image_views.iter_mut() {
                    v.view_info.image = *self.base;
                    temp_view = v.view_info.to_base();
                    *v.base = self
                        .device
                        .create_image_view(&temp_view, None)
                        .expect("Failed to create image view!");
                    *v.extent = self.image_info.extent.clone();
                }
            }
        }
    }

    pub(crate) fn destroy(&self) {
        self.image_views.iter().for_each(|view| {
            view.destroy();
        });
        unsafe {
            self.device.destroy_image(*self.base, None);
            self.device.free_memory(*self.image_memory, None);
        }
    }

    pub fn copy_from_buffer(
        &mut self,
        buffer: &vk::Buffer,
        layer_count: u32,
        queue: &VlLogicalQueue,
    ) {
        let cmd = queue.begin_single_time_command();
        let aspect_flags = Self::aspect_flags_from_format(self.image_info.format.clone());
        self.transition_image_layout_s(
            cmd.clone(),
            self.image_info.initial_layout,
            vk::ImageLayout::TRANSFER_DST_OPTIMAL,
            aspect_flags.clone(),
        );
        let mut region = [vk::BufferImageCopy {
            image_extent: self.image_info.extent,
            ..Default::default()
        }];
        region[0].image_subresource.aspect_mask = aspect_flags.clone();
        region[0].image_subresource.layer_count = layer_count;
        unsafe {
            self.device.cmd_copy_buffer_to_image(
                cmd.clone(),
                buffer.clone(),
                (*self.base).clone(),
                vk::ImageLayout::TRANSFER_DST_OPTIMAL,
                &region,
            );
        }
        let dest_layout = if self.image_info.initial_layout == vk::ImageLayout::UNDEFINED {
            vk::ImageLayout::GENERAL
        } else {
            self.image_info.initial_layout
        };
        self.transition_image_layout_s(
            cmd.clone(),
            vk::ImageLayout::TRANSFER_DST_OPTIMAL,
            dest_layout,
            aspect_flags,
        );
        self.image_info.initial_layout = dest_layout;
        queue.end_single_time_command(cmd.clone());
    }

    pub fn aspect_flags_from_format(format: vk::Format) -> vk::ImageAspectFlags {
        use vk::{Format as F, ImageAspectFlags as A};

        match format {
            // Depth only
            F::D16_UNORM | F::X8_D24_UNORM_PACK32 | F::D32_SFLOAT => A::DEPTH,

            // Stencil only
            F::S8_UINT => A::STENCIL,

            // Depth + Stencil
            F::D16_UNORM_S8_UINT | F::D24_UNORM_S8_UINT | F::D32_SFLOAT_S8_UINT => {
                A::DEPTH | A::STENCIL
            }

            // Default: assume color
            _ => A::COLOR,
        }
    }

    pub fn transition_image_layout_q(
        &mut self,
        queue: &VlLogicalQueue,
        old_layout: vk::ImageLayout,
        new_layout: vk::ImageLayout,
        aspect_mask: vk::ImageAspectFlags,
    ) {
        let cmd = queue.begin_single_time_command();
        self.transition_image_layout_s(cmd, old_layout, new_layout, aspect_mask);
        queue.end_single_time_command(cmd);
    }

    pub fn transition_image_layout_s(
        &mut self,
        command_buffer: vk::CommandBuffer,
        old_layout: vk::ImageLayout,
        new_layout: vk::ImageLayout,
        aspect_mask: vk::ImageAspectFlags,
    ) {
        let (array_layers, mip_levels) =
            if self.image_info.mip_levels > 0 && self.image_info.array_layers > 0 {
                (self.image_info.array_layers, self.image_info.mip_levels)
            } else {
                (1, 1)
            };
        unsafe {
            Self::transition_image_layout(
                &self.device,
                command_buffer,
                (*self.base).clone(),
                old_layout,
                new_layout,
                aspect_mask,
                array_layers,
                mip_levels,
            );
            self.image_info.initial_layout = new_layout;
        }
    }

    pub fn transition_image_layout(
        device: &ash::Device,
        command_buffer: vk::CommandBuffer,
        image: vk::Image,
        old_layout: vk::ImageLayout,
        new_layout: vk::ImageLayout,
        aspect_mask: vk::ImageAspectFlags,
        mip_levels: u32,
        layer_count: u32,
    ) {
        // Helper: map layout pair -> (src_access, dst_access, src_stage, dst_stage)
        fn access_and_stage_for_transition(
            old_layout: vk::ImageLayout,
            new_layout: vk::ImageLayout,
        ) -> (
            vk::AccessFlags,
            vk::AccessFlags,
            vk::PipelineStageFlags,
            vk::PipelineStageFlags,
        ) {
            use vk::{AccessFlags as A, ImageLayout as L, PipelineStageFlags as S};

            match (old_layout, new_layout) {
                // Undefined -> transfer destination (commonly for initial upload)
                (L::UNDEFINED, L::TRANSFER_DST_OPTIMAL) => {
                    (A::empty(), A::TRANSFER_WRITE, S::TOP_OF_PIPE, S::TRANSFER)
                }

                // Transfer destination -> shader read (e.g. after uploading then sampling)
                (L::TRANSFER_DST_OPTIMAL, L::SHADER_READ_ONLY_OPTIMAL) => (
                    A::TRANSFER_WRITE,
                    A::SHADER_READ,
                    S::TRANSFER,
                    S::FRAGMENT_SHADER,
                ),

                // Undefined -> depth/stencil attachment optimal (initial depth)
                (L::UNDEFINED, L::DEPTH_STENCIL_ATTACHMENT_OPTIMAL) => (
                    A::empty(),
                    A::DEPTH_STENCIL_ATTACHMENT_READ | A::DEPTH_STENCIL_ATTACHMENT_WRITE,
                    S::TOP_OF_PIPE,
                    S::EARLY_FRAGMENT_TESTS,
                ),

                // Color attachment optimal -> present (example)
                (L::COLOR_ATTACHMENT_OPTIMAL, L::PRESENT_SRC_KHR) => (
                    A::COLOR_ATTACHMENT_WRITE,
                    A::empty(),
                    S::COLOR_ATTACHMENT_OUTPUT,
                    S::BOTTOM_OF_PIPE,
                ),

                // General case: be conservative: ensure shader reads/writes are visible in fragment stage
                (old, _) => {
                    // Conservative default: writer -> shader read
                    let src_access = if old == L::UNDEFINED {
                        A::empty()
                    } else {
                        // assume writes may have occurred
                        A::MEMORY_WRITE
                            | A::TRANSFER_WRITE
                            | A::COLOR_ATTACHMENT_WRITE
                            | A::DEPTH_STENCIL_ATTACHMENT_WRITE
                    };
                    let dst_access = A::SHADER_READ | A::SHADER_WRITE;
                    let src_stage = S::TOP_OF_PIPE;
                    let dst_stage = S::FRAGMENT_SHADER;
                    (src_access, dst_access, src_stage, dst_stage)
                }
            }
        }

        let (src_access_mask, dst_access_mask, src_stage_mask, dst_stage_mask) =
            access_and_stage_for_transition(old_layout, new_layout);

        let subresource_range = vk::ImageSubresourceRange {
            aspect_mask,
            base_mip_level: 0,
            level_count: mip_levels,
            base_array_layer: 0,
            layer_count,
        };

        let barrier = vk::ImageMemoryBarrier::default()
            .old_layout(old_layout)
            .new_layout(new_layout)
            .src_queue_family_index(vk::QUEUE_FAMILY_IGNORED)
            .dst_queue_family_index(vk::QUEUE_FAMILY_IGNORED)
            .image(image)
            .subresource_range(subresource_range)
            .src_access_mask(src_access_mask)
            .dst_access_mask(dst_access_mask);

        // Issue the barrier. This is unsafe because we call into Vulkan.
        unsafe {
            device.cmd_pipeline_barrier(
                command_buffer,
                src_stage_mask,
                dst_stage_mask,
                vk::DependencyFlags::empty(),
                &[],                            // memory barriers
                &[],                            // buffer memory barriers
                std::slice::from_ref(&barrier), // image memory barriers
            );
        }
    }

    pub fn image_info(&self) -> ImageCreateInfoOwned {
        self.image_info
    }
}
