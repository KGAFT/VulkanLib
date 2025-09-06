use ash::ext::debug_utils;
use ash::vk::DebugUtilsMessengerEXT;
use ash::{vk, Entry, Instance};
use std::borrow::Cow;
use std::ffi;
use std::ffi::c_void;
use std::ops::DerefMut;
use std::sync::{Arc, Mutex};

#[derive(Clone)]
pub struct VlDebugMessenger {
    active: bool,
    debug_utils_loader: debug_utils::Instance,
    debug_messenger: DebugUtilsMessengerEXT,
    debug_callbacks: Vec<
        Arc<
            Mutex<
                dyn FnMut(
                    vk::DebugUtilsMessageSeverityFlagsEXT,
                    vk::DebugUtilsMessageTypeFlagsEXT,
                    Cow<str>,
                    Cow<str>,
                ),
            >,
        >,
    >,
}

impl VlDebugMessenger {
    pub fn new(entry: &Entry, instance: &Instance, initial_callbacks: Vec<
        Arc<
            Mutex<
                dyn FnMut(
                    vk::DebugUtilsMessageSeverityFlagsEXT,
                    vk::DebugUtilsMessageTypeFlagsEXT,
                    Cow<str>,
                    Cow<str>,
                ),
            >,
        >,
    >) -> Self {
        let debug_utils_loader = debug_utils::Instance::new(entry, instance);
        let mut res = Self {
            active: false,
            debug_utils_loader: debug_utils_loader,
            debug_messenger: DebugUtilsMessengerEXT::null(),
            debug_callbacks: initial_callbacks,
        };
        let debug_info = vk::DebugUtilsMessengerCreateInfoEXT::default()
            .message_severity(
                vk::DebugUtilsMessageSeverityFlagsEXT::ERROR
                    | vk::DebugUtilsMessageSeverityFlagsEXT::WARNING
                    | vk::DebugUtilsMessageSeverityFlagsEXT::INFO
                    | vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE,
            )
            .message_type(
                vk::DebugUtilsMessageTypeFlagsEXT::GENERAL
                    | vk::DebugUtilsMessageTypeFlagsEXT::VALIDATION
                    | vk::DebugUtilsMessageTypeFlagsEXT::PERFORMANCE
                    | vk::DebugUtilsMessageTypeFlagsEXT::DEVICE_ADDRESS_BINDING,
            )
            .pfn_user_callback(Some(vulkan_debug_callback))
            .user_data(&mut res as &mut _ as *mut _ as *mut c_void);

        let debug_messenger = unsafe {
            res.debug_utils_loader
                .create_debug_utils_messenger(&debug_info, None)
        }
        .expect("Unable to create debug utils Messenger");
        res.debug_messenger = debug_messenger;
        res.active = true;
        res
    }

    pub fn dispatch_message(
        &mut self,
        message_severity: vk::DebugUtilsMessageSeverityFlagsEXT,
        message_type: vk::DebugUtilsMessageTypeFlagsEXT,
        message: Cow<str>,
        message_id: Cow<str>,
    ) {
        self.debug_callbacks.iter_mut().for_each(|f| {
            f.lock().unwrap().deref_mut()(
                message_severity,
                message_type,
                message.clone(),
                message_id.clone(),
            );
        })
    }
}

impl Drop for VlDebugMessenger {
    fn drop(&mut self) {
        self.active = false;
        unsafe {
            self.debug_utils_loader
                .destroy_debug_utils_messenger(self.debug_messenger, None);
        }
    }
}

unsafe extern "system" fn vulkan_debug_callback(
    message_severity: vk::DebugUtilsMessageSeverityFlagsEXT,
    message_type: vk::DebugUtilsMessageTypeFlagsEXT,
    p_callback_data: *const vk::DebugUtilsMessengerCallbackDataEXT<'_>,
    _user_data: *mut std::os::raw::c_void,
) -> vk::Bool32 {
    let self_ref = _user_data as *mut VlDebugMessenger;
    let self_ref = self_ref.as_mut().unwrap();
    if self_ref.active {
        let callback_data = *p_callback_data;
        let message_id_number = callback_data.message_id_number;

        let message_id_name = if callback_data.p_message_id_name.is_null() {
            Cow::from("")
        } else {
            ffi::CStr::from_ptr(callback_data.p_message_id_name).to_string_lossy()
        };

        let message = if callback_data.p_message.is_null() {
            Cow::from("")
        } else {
            ffi::CStr::from_ptr(callback_data.p_message).to_string_lossy()
        };

        self_ref.dispatch_message(message_severity, message_type, message, message_id_name);
    }

    vk::FALSE
}
