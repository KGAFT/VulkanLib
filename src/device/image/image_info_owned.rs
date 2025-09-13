use std::ffi::c_void;
use ash::vk;
use ash::vk::{Extent3D, Format, ImageCreateFlags, ImageLayout, ImageTiling, ImageType, ImageUsageFlags, SampleCountFlags, SharingMode, StructureType};
#[repr(C)]
#[derive(Copy, Clone, Default)]
pub struct ImageCreateInfoOwned {
    pub s_type: StructureType,
    pub p_next: *const c_void,
    pub flags: ImageCreateFlags,
    pub image_type: ImageType,
    pub format: Format,
    pub extent: Extent3D,
    pub mip_levels: u32,
    pub array_layers: u32,
    pub samples: SampleCountFlags,
    pub tiling: ImageTiling,
    pub usage: ImageUsageFlags,
    pub sharing_mode: SharingMode,
    pub queue_family_index_count: u32,
    pub p_queue_family_indices: *const u32,
    pub initial_layout: ImageLayout
}

impl ImageCreateInfoOwned {
    pub fn from_base(base: vk::ImageCreateInfo) -> Self {
        Self{
            s_type: base.s_type,
            p_next: base.p_next,
            flags: base.flags,
            image_type: base.image_type,
            format: base.format,
            extent: base.extent,
            mip_levels: base.mip_levels,
            array_layers: base.array_layers,
            samples: base.samples,
            tiling: base.tiling,
            usage: base.usage,
            sharing_mode: base.sharing_mode,
            queue_family_index_count: base.queue_family_index_count,
            p_queue_family_indices: base.p_queue_family_indices,
            initial_layout: base.initial_layout,
        }
    }
    pub fn to_base(&self) -> vk::ImageCreateInfo {
        vk::ImageCreateInfo{
            s_type: self.s_type,
            p_next: self.p_next,
            flags: self.flags,
            image_type: self.image_type,
            format: self.format,
            extent: self.extent,
            mip_levels: self.mip_levels,
            array_layers: self.array_layers,
            samples: self.samples,
            tiling: self.tiling,
            usage: self.usage,
            sharing_mode: self.sharing_mode,
            queue_family_index_count: self.queue_family_index_count,
            p_queue_family_indices: self.p_queue_family_indices,
            initial_layout: self.initial_layout,
            ..Default::default()
        }
    }
}