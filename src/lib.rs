#![allow(dead_code)]
mod device;
mod instance;
pub mod util;
mod window;

#[cfg(test)]
mod tests {
    use crate::device::device_builder::VlDeviceBuilder;
    use crate::device::logical_device::logical_device::VlLogicalDevice;
    use crate::device::physical_device::device_suitability::VlDeviceSuitability;
    use crate::device::physical_device::physical_device::VlPhysicalDevice;
    use crate::instance::instance::VlInstance;
    use crate::instance::instance_builder::VlInstanceBuilder;
    use crate::window::Window;
    use ash::vk;
    use std::borrow::Cow;
    use std::ffi::CString;
    use std::io::Write;
    use std::sync::{Arc, Mutex};

    use crate::device::swapchain::VlSwapChain;

    #[test]
    fn it_works() {
        let mut window = Window::new(800, 600).unwrap();

        let mut builder = VlInstanceBuilder::new();
        builder.set_app_name(CString::new("hello app name").unwrap());
        builder.set_engine_name(CString::new("Vulkan Engine").unwrap());
        let mut exts = Window::get_required_instance_extensions().unwrap();
        while let Some(ext) = exts.pop() {
            builder.add_enabled_extension(ext);
        }

        builder.enable_present_supported();
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

        let mut devices = VlPhysicalDevice::enumerate(instance.get_instance_r());
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
        let surface = window
            .surface(
                instance.surface_loader().unwrap().clone(),
                instance.get_instance_r(),
            )
            .expect("Failed to create Vulkan surface");
        dev_builder.require_graphics();
        dev_builder.require_compute();
        dev_builder.require_raytracing();
        dev_builder.enable_presentation(surface.clone());

        let suit = VlDeviceSuitability::is_device_suitable(&instance, &dev_builder, &devices[0]);
        if suit.0 {
            let device =
                VlLogicalDevice::new(&instance, devices.pop().unwrap(), &dev_builder, suit);
            println!("{:?}", device.find_depth_format());
            let swap_chain = VlSwapChain::new(device.clone(), instance.clone(), surface.clone(), 800, 600, true);

        }

        while !window.need_close() {
            window.poll_events();
        }
        drop(window);
    }
}
