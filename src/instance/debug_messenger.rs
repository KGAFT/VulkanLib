use ash::ext::debug_utils;
use ash::vk::DebugUtilsMessengerEXT;
use ash::{vk, Entry, Instance};
use std::borrow::Cow;
use std::ffi;
use std::ops::DerefMut;
use std::sync::{Arc, Mutex};

pub type DebugCall = dyn FnMut(
    i32,
    vk::DebugUtilsMessageSeverityFlagsEXT,
    vk::DebugUtilsMessageTypeFlagsEXT,
    Cow<str>,
    Cow<str>,
);
#[repr(C)]
#[derive(Clone)]
struct CbContainer {
    debug_callbacks: Vec<Arc<Mutex<DebugCall>>>,
}

#[derive(Clone)]
#[repr(C)]
pub struct VlDebugMessenger {
    active: bool,
    debug_utils_loader: debug_utils::Instance,
    debug_messenger: DebugUtilsMessengerEXT,
    debug_callbacks: *mut CbContainer,
}

impl VlDebugMessenger {
    pub fn new(
        entry: &Entry,
        instance: &Instance,
        mut initial_callbacks: Vec<Arc<Mutex<DebugCall>>>,
    ) -> Self {
        let debug_utils_loader = debug_utils::Instance::new(entry, instance);
        let mut cbs = Vec::with_capacity(initial_callbacks.len());
        while let Some(callback) = initial_callbacks.pop() {
            cbs.push(callback);
        }

        let mut res = Self {
            active: true,
            debug_utils_loader,
            debug_messenger: DebugUtilsMessengerEXT::null(),
            debug_callbacks: Box::into_raw(Box::from(CbContainer {
                debug_callbacks: cbs,
            })),
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
            .user_data(res.debug_callbacks.clone() as *mut _);

        let debug_messenger = unsafe {
            res.debug_utils_loader
                .create_debug_utils_messenger(&debug_info, None)
        }
        .expect("Unable to create debug utils Messenger");
        res.debug_messenger = debug_messenger;
        res
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
    let self_ref = _user_data as *mut Vec<Arc<Mutex<DebugCall>>>;
    let self_ref = self_ref.as_mut().unwrap();

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
    self_ref.iter_mut().for_each(|f| {
        f.lock().unwrap().deref_mut()(
            message_id_number,
            message_severity,
            message_type,
            message.clone(),
            message_id_name.clone(),
        );
    });

    vk::FALSE
}
