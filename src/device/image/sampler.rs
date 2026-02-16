use ash::vk;
use crate::device::logical_device::logical_device::VlLogicalDevice;

pub struct VlSampler {
    sampler: vk::Sampler,
    device: ash::Device,
}

impl VlSampler {
    pub fn new(device: &VlLogicalDevice) -> Self {
        let device_handle = device.device();

        let create_info = vk::SamplerCreateInfo::default()
            .mag_filter(vk::Filter::LINEAR)
            .min_filter(vk::Filter::LINEAR)
            .address_mode_u(vk::SamplerAddressMode::REPEAT)
            .address_mode_v(vk::SamplerAddressMode::REPEAT)
            .address_mode_w(vk::SamplerAddressMode::REPEAT)
            .anisotropy_enable(false)
            .max_anisotropy(
                device.base_device_r().properties()
                    .limits
                    .max_sampler_anisotropy,
            )
            .border_color(vk::BorderColor::INT_OPAQUE_BLACK)
            .unnormalized_coordinates(false)
            .compare_enable(false)
            .compare_op(vk::CompareOp::ALWAYS)
            .mipmap_mode(vk::SamplerMipmapMode::LINEAR);

        let sampler = unsafe {
            device_handle
                .create_sampler(&create_info, None)
                .expect("Failed to create sampler")
        };

        Self {
            sampler,
            device: device_handle,
        }
    }

    pub fn from_create_info(
        device: &VlLogicalDevice,
        create_info: &vk::SamplerCreateInfo,
    ) -> Self {
        let device_handle = device.device();

        let sampler = unsafe {
            device_handle
                .create_sampler(create_info, None)
                .expect("Failed to create sampler")
        };

        Self {
            sampler,
            device: device_handle,
        }
    }

    pub fn sampler(&self) -> vk::Sampler {
        self.sampler
    }
}


impl Drop for VlSampler {
    fn drop(&mut self) {
        unsafe { self.device.destroy_sampler(self.sampler, None) }
    }
}