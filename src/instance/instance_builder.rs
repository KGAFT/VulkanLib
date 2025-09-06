use std::borrow::Cow;
use std::ffi::{c_char, CStr, CString};
use std::sync::{Arc, Mutex};
use ash::vk;

pub struct VlInstanceBuilder {
    enabled_layers: Vec<CString>,
    enabled_extensions: Vec<CString>,
    debug_enabled: bool,
    app_name: Option<CString>,
    engine_name: Option<CString>,
    initial_callbacks: Vec<
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

impl VlInstanceBuilder {
    pub fn new() -> Self {
        let mut res = Self {
            enabled_extensions: Vec::new(),
            enabled_layers: Vec::new(),
            debug_enabled: false,
            app_name: None,
            engine_name: None,
            initial_callbacks: Vec::new(),
        };
        #[cfg(any(target_os = "macos", target_os = "ios"))]
        {
            res.enabled_extensions.push(ash::khr::portability_enumeration::NAME.as_ptr());
            // Enabling this extension is a requirement when using `VK_KHR_portability_subset`
            res.enabled_extensions.push(ash::khr::get_physical_device_properties2::NAME.as_ptr());
        }
        res
    }
    pub fn add_enabled_layer(&mut self, enabled_layer: CString) {
        self.enabled_layers.push(enabled_layer);
    }
    pub fn add_enabled_extension(&mut self, enabled_extension: CString) {
        self.enabled_extensions.push(enabled_extension);
    }
    pub fn preset_for_debug(&mut self) {
        self.enabled_layers
            .push(CString::new("VK_LAYER_KHRONOS_validation").unwrap());
        self.enabled_extensions
            .push(CString::new("VK_EXT_debug_utils").unwrap());

        self.debug_enabled = true;
    }

    pub fn initial_callbacks(&mut self) ->  Vec<
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
    >{
        let mut res = Vec::new();
        while let Some(callback) = self.initial_callbacks.pop() {
            res.push(callback);
        }
        res
    }

    pub fn add_initial_debug_callback(&mut self,  callback: Arc<Mutex<dyn FnMut(
        vk::DebugUtilsMessageSeverityFlagsEXT,
        vk::DebugUtilsMessageTypeFlagsEXT,
        Cow<str>,
        Cow<str>,
    )>>){
        self.initial_callbacks.push(callback);
    }

    pub fn set_app_name(&mut self, app_name: CString) {
        self.app_name = Some(app_name);
    }

    pub fn set_engine_name(&mut self, engine_name: CString) {
        self.engine_name = Some(engine_name);
    }

    pub fn enabled_layers(&self) -> Vec<*const c_char> {
        self.enabled_layers.iter().map(|s| s.as_ptr()).collect()
    }

    pub fn enabled_extensions(&self) -> Vec<*const c_char> {
        self.enabled_extensions.iter().map(|e| e.as_ptr()).collect()
    }

    pub fn c_enabled_layers(&self) -> Vec<CString> {
        self.enabled_layers.clone()
    }
    pub fn c_enabled_extensions(&self) -> Vec<CString> {
        self.enabled_extensions.clone()
    }
    pub fn debug_enabled(&self) -> bool {
        self.debug_enabled
    }

    pub fn app_name(&self) -> &Option<CString> {
        &self.app_name
    }

    pub fn engine_name(&self) -> &Option<CString> {
        &self.engine_name
    }
}
