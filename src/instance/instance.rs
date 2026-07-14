use crate::instance::debug_messenger::VlDebugMessenger;
use crate::instance::instance_builder::VlInstanceBuilder;
use ash::khr::surface;
use ash::{vk, Entry};
use std::ffi::CString;

pub struct VlInstance {
    entry: Entry,
    instance: ash::Instance,
    surface_loader: Option<surface::Instance>,
    enabled_layers: Vec<CString>,
    enabled_extensions: Vec<CString>,
    debug_messenger: Option<VlDebugMessenger>,
    original: bool,
}

impl Clone for VlInstance {
    fn clone(&self) -> Self {
        let res = Self {
            entry: self.entry.clone(),
            instance: self.instance.clone(),
            surface_loader: self.surface_loader.clone(),
            enabled_layers: vec![],
            enabled_extensions: vec![],
            debug_messenger: self.debug_messenger.clone(),
            original: false,
        };
        res
    }
}

impl VlInstance {
    pub fn new(mut builder: VlInstanceBuilder) -> Result<Self, vk::Result> {
        let entry = Entry::linked();
        let mut app_info = vk::ApplicationInfo {
            api_version: vk::API_VERSION_1_3,
            ..Default::default()
        };
        if let Some(app_name) = builder.app_name() {
            app_info = app_info.application_name(app_name.as_c_str());
        }
        if let Some(engine_name) = builder.engine_name() {
            app_info = app_info.engine_name(engine_name.as_c_str());
        }
        let enabled_layers = builder.enabled_layers();
        let enabled_extensions = builder.enabled_extensions();
        let create_info = vk::InstanceCreateInfo {
            ..Default::default()
        }
        .application_info(&app_info)
        .enabled_layer_names(&enabled_layers)
        .enabled_extension_names(&enabled_extensions);
        let instance = unsafe { entry.create_instance(&create_info, None) }?;
        let debug_messenger = if builder.debug_enabled() {
            Some(VlDebugMessenger::new(
                &entry,
                &instance,
                builder.initial_callbacks(),
            )?)
        } else {
            None
        };
        let surface_loader = if builder.present_enabled() {
            Some(surface::Instance::new(&entry, &instance))
        } else {
            None
        };
        Ok(Self {
            entry,
            instance,
            enabled_layers: builder.c_enabled_layers(),
            enabled_extensions: builder.c_enabled_extensions(),
            debug_messenger,
            surface_loader,
            original: true,
        })
    }

    pub fn get_instance_r(&self) -> &ash::Instance {
        &self.instance
    }
    pub fn get_instance(&self) -> ash::Instance {
        self.instance.clone()
    }
    pub fn get_loader_r(&self) -> &Entry {
        &self.entry
    }
    pub fn get_entry(&self) -> Entry {
        self.entry.clone()
    }
    pub fn surface_loader(&self) -> Option<&surface::Instance> {
        self.surface_loader.as_ref()
    }

    pub fn enabled_layers(&self) -> &Vec<CString> {
        &self.enabled_layers
    }
}

impl Drop for VlInstance {
    fn drop(&mut self) {
        if self.original {
            unsafe {
                if self.debug_messenger.is_some() {
                    let msg = self.debug_messenger.take().unwrap();
                    std::mem::drop(msg);
                }
                if self.surface_loader.is_some() {
                    let surface_loader = self.surface_loader.take().unwrap();
                    std::mem::drop(surface_loader);
                }
                self.instance.destroy_instance(None);
            }
        }
    }
}
