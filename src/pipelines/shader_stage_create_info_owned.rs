use ash::vk;
use std::ffi::CString;

pub struct PipelineShaderStageCreateInfoOwned {
    pub s_type: vk::StructureType,
    pub p_next: Option<*const std::ffi::c_void>,
    pub flags: vk::PipelineShaderStageCreateFlags,
    pub stage: vk::ShaderStageFlags,
    pub module: vk::ShaderModule,
    pub name: CString,
}

impl PipelineShaderStageCreateInfoOwned {
    pub fn as_vulkan(&self) -> vk::PipelineShaderStageCreateInfo {
        vk::PipelineShaderStageCreateInfo {
            s_type: self.s_type,
            p_next: self.p_next.unwrap_or(std::ptr::null()),
            flags: self.flags,
            stage: self.stage,
            module: self.module,
            p_name: self.name.as_ptr(),
           ..Default::default()
        }
    }

    pub unsafe fn from_vulkan(info: &vk::PipelineShaderStageCreateInfo) -> Self {
        let name = CString::from(std::ffi::CStr::from_ptr(info.p_name));
        Self {
            s_type: info.s_type,
            p_next: if info.p_next.is_null() { None } else { Some(info.p_next) },
            flags: info.flags,
            stage: info.stage,
            module: info.module,
            name,
        }
    }
}
