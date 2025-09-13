use crate::device::image::image_view_info_owned::ImageViewCreateInfoOwned;
use ash::vk;

pub struct VlImageView {
    pub(crate) base: *mut vk::ImageView,
    pub(crate) extent: *mut vk::Extent3D,
    pub(crate) view_info: ImageViewCreateInfoOwned,
    device: ash::Device,
    origin: bool,
}

impl Clone for VlImageView {
    fn clone(&self) -> Self {
        Self {
            base: self.base.clone(),
            extent: self.extent.clone(),
            view_info: self.view_info.clone(),
            device: self.device.clone(),
            origin: false,
        }
    }
}

impl Drop for VlImageView {
    fn drop(&mut self) {
        if self.origin {
            unsafe {
                self.destroy();
            }
        }
    }
}

impl VlImageView {
    pub fn new(
        extent: vk::Extent3D,
        device: ash::Device,
        base: vk::ImageView,
        info: &vk::ImageViewCreateInfo,
    ) -> Self {
        Self {
            device,
            extent: Box::into_raw(Box::from(extent)),
            view_info: ImageViewCreateInfoOwned::from_base(info),
            base: Box::into_raw(Box::from(base)),
            origin: true,
        }
    }

    pub fn base(&self) -> vk::ImageView {
        unsafe { (*self.base).clone() }
    }



    pub(crate) fn destroy(&self) {
        unsafe { self.device.destroy_image_view(*self.base, None); }
    }

    pub fn extent(&self) -> vk::Extent3D {
        unsafe{
            *self.extent
        }
    }

    pub fn view_info(&self) -> &ImageViewCreateInfoOwned {
        &self.view_info
    }
}
