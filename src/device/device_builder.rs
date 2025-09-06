use std::ffi::CString;
use ash::vk;

pub struct VlDeviceBuilder{
    request_extension: Vec<CString>,
    require_present: vk::SurfaceKHR,
    require_graphics: bool,
    require_raytracing: bool,
    require_compute: bool,
}

impl VlDeviceBuilder {

}