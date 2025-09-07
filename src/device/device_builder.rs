use ash::vk;
use std::ffi::{c_char, CString};
use crate::util::c_string_vec_to_ptr;

#[derive(Default)]
pub struct VlDeviceBuilder {
    request_extension: Vec<CString>,
    require_present: vk::SurfaceKHR,
    require_graphics: bool,
    require_raytracing: bool,
    require_compute: bool,
}

impl VlDeviceBuilder {

    pub fn new() -> Self {
        Self{..Default::default()}
    }
    pub fn add_extension(&mut self, extension: CString) {
        self.request_extension.push(extension);
    }

    pub fn enable_presentation(&mut self, surface: vk::SurfaceKHR) {
        self.require_present = surface;
    }

    pub fn enable_raytracing(&mut self) {
        self.request_extension
            .push(CString::new("VK_KHR_acceleration_structure").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_ray_tracing_pipeline").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_pipeline_library").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_deferred_host_operations").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_spirv_1_4").unwrap());
        self.request_extension
            .push(CString::new("VK_EXT_descriptor_indexing").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_buffer_device_address").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_shader_float_controls").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_maintenance3").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_device_group").unwrap());
        self.require_raytracing = true;
    }

    pub fn enable_graphics(&mut self) {
        self.request_extension
            .push(CString::new("VK_KHR_dynamic_rendering").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_depth_stencil_resolve").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_create_renderpass2").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_multiview").unwrap());
        self.request_extension
            .push(CString::new("VK_KHR_maintenance2").unwrap());
        self.require_graphics = true;
    }

    pub fn get_extensions(&self) -> Vec<*const c_char> {
        c_string_vec_to_ptr(&self.request_extension)
    }

    pub fn enable_compute(&mut self) {
        self.require_compute = true;
    }

    pub fn require_present(&self) -> vk::SurfaceKHR {
        self.require_present
    }

    pub fn require_graphics(&self) -> bool {
        self.require_graphics
    }

    pub fn require_raytracing(&self) -> bool {
        self.require_raytracing
    }

    pub fn require_compute(&self) -> bool {
        self.require_compute
    }

    pub fn request_extension(&self) -> &Vec<CString> {
        &self.request_extension
    }
}
