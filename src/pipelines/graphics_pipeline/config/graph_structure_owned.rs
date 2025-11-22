use std::ffi::c_void;
use ash::vk;
use ash::vk::{Bool32, CullModeFlags, FrontFace, PipelineInputAssemblyStateCreateFlags, PipelineInputAssemblyStateCreateInfo, PipelineRasterizationStateCreateFlags, PipelineRasterizationStateCreateInfo, PolygonMode, PrimitiveTopology, StructureType};


#[derive(Clone, Copy, Default)]
pub struct PipelineInputAssemblyStateCreateInfoOwned {
    pub s_type: StructureType,
    pub p_next: *const c_void,
    pub flags: PipelineInputAssemblyStateCreateFlags,
    pub topology: PrimitiveTopology,
    pub primitive_restart_enable: Bool32,
}

impl PipelineInputAssemblyStateCreateInfoOwned {
    fn from(info: &PipelineInputAssemblyStateCreateInfo) -> Self {
        Self{
            s_type: info.s_type,
            p_next: info.p_next,
            flags: info.flags,
            topology: info.topology,
            primitive_restart_enable: info.primitive_restart_enable,
        }
    }

    pub fn to_base(&self) -> PipelineInputAssemblyStateCreateInfo {
        PipelineInputAssemblyStateCreateInfo{
            s_type: self.s_type,
            p_next: self.p_next,
            flags: self.flags,
            topology: self.topology,
            primitive_restart_enable: self.primitive_restart_enable,
            _marker: Default::default(),
        }
    }
}
#[derive(Clone, Copy, Default)]
pub struct PipelineRasterizationStateCreateInfoOwned {
    pub s_type: vk::StructureType,
    pub p_next: *const std::ffi::c_void,
    pub flags: vk::PipelineRasterizationStateCreateFlags,
    pub depth_clamp_enable: vk::Bool32,
    pub rasterizer_discard_enable: vk::Bool32,
    pub polygon_mode: vk::PolygonMode,
    pub cull_mode: vk::CullModeFlags,
    pub front_face: vk::FrontFace,
    pub depth_bias_enable: vk::Bool32,
    pub depth_bias_constant_factor: f32,
    pub depth_bias_clamp: f32,
    pub depth_bias_slope_factor: f32,
    pub line_width: f32,
}

impl PipelineRasterizationStateCreateInfoOwned {
    pub fn from(info: &vk::PipelineRasterizationStateCreateInfo) -> Self {
        Self {
            s_type: info.s_type,
            p_next: info.p_next,
            flags: info.flags,
            depth_clamp_enable: info.depth_clamp_enable,
            rasterizer_discard_enable: info.rasterizer_discard_enable,
            polygon_mode: info.polygon_mode,
            cull_mode: info.cull_mode,
            front_face: info.front_face,
            depth_bias_enable: info.depth_bias_enable,
            depth_bias_constant_factor: info.depth_bias_constant_factor,
            depth_bias_clamp: info.depth_bias_clamp,
            depth_bias_slope_factor: info.depth_bias_slope_factor,
            line_width: info.line_width,
        }
    }

    pub fn to_base(&self) -> vk::PipelineRasterizationStateCreateInfo {
        vk::PipelineRasterizationStateCreateInfo {
            s_type: self.s_type,
            p_next: self.p_next,
            flags: self.flags,
            depth_clamp_enable: self.depth_clamp_enable,
            rasterizer_discard_enable: self.rasterizer_discard_enable,
            polygon_mode: self.polygon_mode,
            cull_mode: self.cull_mode,
            front_face: self.front_face,
            depth_bias_enable: self.depth_bias_enable,
            depth_bias_constant_factor: self.depth_bias_constant_factor,
            depth_bias_clamp: self.depth_bias_clamp,
            depth_bias_slope_factor: self.depth_bias_slope_factor,
            line_width: self.line_width,
            ..Default::default() // fills the PhantomData
        }
    }
}

#[derive(Clone, Copy, Default)]
pub struct PipelineMultisampleStateCreateInfoOwned {
    pub s_type: vk::StructureType,
    pub p_next: *const std::ffi::c_void,
    pub flags: vk::PipelineMultisampleStateCreateFlags,
    pub rasterization_samples: vk::SampleCountFlags,
    pub sample_shading_enable: vk::Bool32,
    pub min_sample_shading: f32,
    pub p_sample_mask: *const vk::SampleMask,
    pub alpha_to_coverage_enable: vk::Bool32,
    pub alpha_to_one_enable: vk::Bool32,
}

impl PipelineMultisampleStateCreateInfoOwned {
    pub fn from(info: &vk::PipelineMultisampleStateCreateInfo) -> Self {
        Self {
            s_type: info.s_type,
            p_next: info.p_next,
            flags: info.flags,
            rasterization_samples: info.rasterization_samples,
            sample_shading_enable: info.sample_shading_enable,
            min_sample_shading: info.min_sample_shading,
            p_sample_mask: info.p_sample_mask,
            alpha_to_coverage_enable: info.alpha_to_coverage_enable,
            alpha_to_one_enable: info.alpha_to_one_enable,
        }
    }

    pub fn to_base(&self) -> vk::PipelineMultisampleStateCreateInfo {
        vk::PipelineMultisampleStateCreateInfo {
            s_type: self.s_type,
            p_next: self.p_next,
            flags: self.flags,
            rasterization_samples: self.rasterization_samples,
            sample_shading_enable: self.sample_shading_enable,
            min_sample_shading: self.min_sample_shading,
            p_sample_mask: self.p_sample_mask,
            alpha_to_coverage_enable: self.alpha_to_coverage_enable,
            alpha_to_one_enable: self.alpha_to_one_enable,
            ..Default::default() // fills PhantomData
        }
    }
}

#[derive(Clone, Copy, Default)]
pub struct PipelineColorBlendStateCreateInfoOwned {
    pub s_type: vk::StructureType,
    pub p_next: *const std::ffi::c_void,
    pub flags: vk::PipelineColorBlendStateCreateFlags,
    pub logic_op_enable: vk::Bool32,
    pub logic_op: vk::LogicOp,
    pub attachment_count: u32,
    pub p_attachments: *const vk::PipelineColorBlendAttachmentState,
    pub blend_constants: [f32; 4],
}

impl PipelineColorBlendStateCreateInfoOwned {
    pub fn from(info: &vk::PipelineColorBlendStateCreateInfo) -> Self {
        Self {
            s_type: info.s_type,
            p_next: info.p_next,
            flags: info.flags,
            logic_op_enable: info.logic_op_enable,
            logic_op: info.logic_op,
            attachment_count: info.attachment_count,
            p_attachments: info.p_attachments,
            blend_constants: info.blend_constants,
        }
    }

    pub fn to_base(&self) -> vk::PipelineColorBlendStateCreateInfo {
        vk::PipelineColorBlendStateCreateInfo {
            s_type: self.s_type,
            p_next: self.p_next,
            flags: self.flags,
            logic_op_enable: self.logic_op_enable,
            logic_op: self.logic_op,
            attachment_count: self.attachment_count,
            p_attachments: self.p_attachments,
            blend_constants: self.blend_constants,
            ..Default::default() // fills PhantomData
        }
    }
}

#[derive(Clone, Copy, Default)]
pub struct PipelineDepthStencilStateCreateInfoOwned {
    pub s_type: vk::StructureType,
    pub p_next: *const std::ffi::c_void,
    pub flags: vk::PipelineDepthStencilStateCreateFlags,
    pub depth_test_enable: vk::Bool32,
    pub depth_write_enable: vk::Bool32,
    pub depth_compare_op: vk::CompareOp,
    pub depth_bounds_test_enable: vk::Bool32,
    pub stencil_test_enable: vk::Bool32,
    pub front: vk::StencilOpState,
    pub back: vk::StencilOpState,
    pub min_depth_bounds: f32,
    pub max_depth_bounds: f32,
}

impl PipelineDepthStencilStateCreateInfoOwned {
    pub fn from(info: &vk::PipelineDepthStencilStateCreateInfo) -> Self {
        Self {
            s_type: info.s_type,
            p_next: info.p_next,
            flags: info.flags,
            depth_test_enable: info.depth_test_enable,
            depth_write_enable: info.depth_write_enable,
            depth_compare_op: info.depth_compare_op,
            depth_bounds_test_enable: info.depth_bounds_test_enable,
            stencil_test_enable: info.stencil_test_enable,
            front: info.front,
            back: info.back,
            min_depth_bounds: info.min_depth_bounds,
            max_depth_bounds: info.max_depth_bounds,
        }
    }

    pub fn to_base(&self) -> vk::PipelineDepthStencilStateCreateInfo {
        vk::PipelineDepthStencilStateCreateInfo {
            s_type: self.s_type,
            p_next: self.p_next,
            flags: self.flags,
            depth_test_enable: self.depth_test_enable,
            depth_write_enable: self.depth_write_enable,
            depth_compare_op: self.depth_compare_op,
            depth_bounds_test_enable: self.depth_bounds_test_enable,
            stencil_test_enable: self.stencil_test_enable,
            front: self.front,
            back: self.back,
            min_depth_bounds: self.min_depth_bounds,
            max_depth_bounds: self.max_depth_bounds,
            ..Default::default() // fills PhantomData
        }
    }
}
