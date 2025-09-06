#![allow(dead_code)]
mod device;
mod instance;

#[cfg(test)]
mod tests {
    use crate::device::device_builder::VlDeviceBuilder;
    use crate::device::physical_device::device_suitability::VlDeviceSuitability;
    use crate::device::physical_device::physical_device::VlPhysicalDevice;
    use crate::instance::instance::VlInstance;
    use crate::instance::instance_builder::VlInstanceBuilder;
    use ash::vk;
    use std::borrow::Cow;
    use std::ffi::CString;
    use std::io::Write;
    use std::sync::{Arc, Mutex};

    #[test]
    fn it_works() {
        let mut builder = VlInstanceBuilder::new();
        builder.set_app_name(CString::new("hello app name").unwrap());
        builder.set_engine_name(CString::new("Vulkan Engine").unwrap());
        builder.add_enabled_extension("VK_KHR_surface".parse().unwrap());

        builder.add_initial_debug_callback(Arc::new(Mutex::new(
            move |id: i32,
                  severity: vk::DebugUtilsMessageSeverityFlagsEXT,
                  m_type: vk::DebugUtilsMessageTypeFlagsEXT,
                  message: Cow<str>,
                  message_id: Cow<str>| {
                let msg = format!("{severity:?}: {m_type:?} [{message_id} {id}] : {message}",);
                println!("{}", msg);
                std::io::stdout().flush().unwrap();
            },
        )));
        builder.preset_for_debug();
        let instance = VlInstance::new(builder);
        let devices = VlPhysicalDevice::enumerate(instance.get_instance_r());
        devices.iter().for_each(|device| {
            println!(
                "{}",
                device
                    .properties()
                    .device_name_as_c_str()
                    .unwrap()
                    .to_str()
                    .unwrap()
            );
        });
        let mut dev_builder = VlDeviceBuilder::new();
        dev_builder.require_graphics();
        dev_builder.require_compute();
        dev_builder.require_raytracing();
        devices.iter().for_each(|device| {
            let suit = VlDeviceSuitability::is_device_suitable(&instance, &dev_builder, device);
            if suit.0 {
                println!("suitable")
            }
        });
        loop {

        }
    }
}
