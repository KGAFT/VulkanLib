mod instance;
mod device;

#[cfg(test)]
mod tests {
    use std::borrow::Cow;
    use std::ffi::CString;
    use std::sync::{Arc, Mutex};
    use ash::vk;
    use crate::instance::instance::VlInstance;
    use crate::instance::instance_builder::VlInstanceBuilder;

    #[test]
    fn it_works() {
        let mut builder = VlInstanceBuilder::new();
        builder.set_app_name(CString::new("hello app name").unwrap());
        builder.set_engine_name(CString::new("Vulkan Engine").unwrap());
        builder.add_enabled_extension("VK_KHR_surface".parse().unwrap());
        builder.add_initial_debug_callback(Arc::new(Mutex::new(
            move |severity: vk::DebugUtilsMessageSeverityFlagsEXT,
                  m_type: vk::DebugUtilsMessageTypeFlagsEXT,
                  message: Cow<str>,
                  message_id: Cow<str>| {
                println!(
                    "{severity:?}:\n{m_type:?} [{message_id} ()] : {message}\n",
                );
            }
        )));
        builder.preset_for_debug();
        let instance = VlInstance::new(builder);
        loop {

        }
    }
}