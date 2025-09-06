use crate::device::device_builder::VlDeviceBuilder;
use crate::device::physical_device::physical_device::VlPhysicalDevice;
use crate::instance::instance::VlInstance;
use ash::vk;
use ash::vk::Handle;

#[derive(Default)]
pub struct VlQueueFamilyInfo {
    index: u32,
    properties: vk::QueueFamilyProperties,
    support_present: bool,
}

pub struct VlDeviceSuitability {}

impl VlDeviceSuitability {
    pub fn is_device_suitable(
        instance: &VlInstance,
        builder: &VlDeviceBuilder,
        device: &VlPhysicalDevice,
    ) -> (bool, Vec<VlQueueFamilyInfo>) {
        for e in builder.request_extension().iter() {
            let mut found = false;
            for x in device.extension_properties().iter() {
                if x.extension_name_as_c_str().unwrap().eq(e.as_c_str()) {
                    found = true;
                    break;
                }
            }
            if !found {
                return (false, vec![]);
            }
        }
        let mut graphics_found = false;
        let mut present_found = false;
        let mut compute_found = false;
        let mut info = VlQueueFamilyInfo {
            ..Default::default()
        };
        let mut queue_counter: u32 = 0;
        let mut pre_res = Vec::new();
        for x in device.queue_properties().iter() {
            if builder.require_graphics() {
                if x.queue_flags.contains(vk::QueueFlags::GRAPHICS) {
                    graphics_found = true;
                    info = VlQueueFamilyInfo {
                        index: queue_counter,
                        properties: x.clone(),
                        support_present: false,
                    };
                }
            }
            if builder.require_compute() {
                if x.queue_flags.contains(vk::QueueFlags::COMPUTE) {
                    compute_found = true;
                    info = VlQueueFamilyInfo {
                        index: queue_counter,
                        properties: x.clone(),
                        support_present: false,
                    };
                }
            }
            unsafe {
                if !builder.require_present().is_null() {
                    let loader = instance.surface_loader();
                    if loader.is_none() {
                        eprintln!(
                            "Surface support required in device, but not enabled in instance!"
                        );
                        return (false, vec![]);
                    }
                    let loader = loader.unwrap();
                    let support = loader.get_physical_device_surface_support(
                        device.inner().clone(),
                        queue_counter,
                        builder.require_present(),
                    );
                    if support.is_err() {
                        eprintln!(
                            "Failed to request surface support: {}",
                            support.unwrap_err()
                        );
                        return (false, vec![]);
                    }
                    let support = support.unwrap();
                    if support {
                        present_found = true;
                        info = VlQueueFamilyInfo {
                            index: queue_counter,
                            properties: x.clone(),
                            support_present: true,
                        };
                    }
                }
            }
            pre_res.push(info);
            info = VlQueueFamilyInfo {
                ..Default::default()
            };
            if (graphics_found == builder.require_graphics()
                && present_found == !builder.require_present().is_null()
                && compute_found == builder.require_compute())
            {
                break;
            }
            queue_counter = queue_counter + 1;
        }
        return (
            graphics_found == builder.require_graphics()
                && present_found == !builder.require_present().is_null()
                && compute_found == builder.require_compute(),
            pre_res,
        );
    }
}
