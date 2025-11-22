use std::ffi::c_void;
use ash::vk;
use ash::vk::{ComponentMapping, Format, Image, ImageSubresourceRange, ImageViewCreateFlags, ImageViewType, StructureType};
#[derive(Clone)]
pub struct ImageViewCreateInfoOwned{
    pub s_type: StructureType,
    pub p_next: *const c_void,
    pub flags: ImageViewCreateFlags,
    pub image: Image,
    pub view_type: ImageViewType,
    pub format: Format,
    pub components: ComponentMapping,
    pub subresource_range: ImageSubresourceRange,
}

impl ImageViewCreateInfoOwned{
    pub fn from_base(base: &vk::ImageViewCreateInfo) -> Self{
        Self{
            s_type: base.s_type.clone(),
            p_next: base.p_next.clone(),
            flags: base.flags.clone(),
            image: base.image.clone(),
            view_type: base.view_type.clone(),
            format: base.format.clone(),
            components: base.components.clone(),
            subresource_range: base.subresource_range.clone(),
        }
    }

    pub fn to_base(&self) -> vk::ImageViewCreateInfo<'_>{
        vk::ImageViewCreateInfo{
            s_type: self.s_type.clone(),
            p_next: self.p_next.clone(),
            flags: self.flags.clone(),
            image: self.image.clone(),
            view_type: self.view_type.clone(),
            format: self.format.clone(),
            components: self.components.clone(),
            subresource_range: self.subresource_range.clone(),
            ..Default::default()
        }
    }
}