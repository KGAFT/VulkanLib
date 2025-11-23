mod graph_structure_owned;
mod graph_pipeline_builder;

use ash::vk;
use crate::pipelines::graphics_pipeline::config::graph_structure_owned::*;

pub struct VlGraphicsPipelineConfig {
    pub viewport: vk::Viewport,
    pub scissor: vk::Rect2D,
    pub input_assembly_info: PipelineInputAssemblyStateCreateInfoOwned,
    pub rasterization_info: PipelineRasterizationStateCreateInfoOwned,
    pub multisample_info: PipelineMultisampleStateCreateInfoOwned,
    pub color_blend_attachments: Vec<vk::PipelineColorBlendAttachmentState>,
    pub color_blend_info:  PipelineColorBlendStateCreateInfoOwned,
    pub depth_stencil_info: PipelineDepthStencilStateCreateInfoOwned,
    pub subpass: u32,
}

impl VlGraphicsPipelineConfig {
    pub fn create_config(
        attachment_count: u32,
        alpha_blending: bool,
        width: u32,
        height: u32,
    ) -> Self {
        // Input assembly
        let input_assembly_info = PipelineInputAssemblyStateCreateInfoOwned {
            s_type: vk::StructureType::PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            p_next: std::ptr::null(),
            flags: vk::PipelineInputAssemblyStateCreateFlags::empty(),
            topology: vk::PrimitiveTopology::TRIANGLE_LIST,
            primitive_restart_enable: vk::FALSE,
        };

        // Viewport & scissor
        let viewport = vk::Viewport {
            x: 0.0,
            y: 0.0,
            width: width as f32,
            height: height as f32,
            min_depth: 0.0,
            max_depth: 1.0,
        };
        let scissor = vk::Rect2D {
            offset: vk::Offset2D { x: 0, y: 0 },
            extent: vk::Extent2D { width, height },
        };

        // Rasterization
        let rasterization_info = PipelineRasterizationStateCreateInfoOwned {
            s_type: vk::StructureType::PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            p_next: std::ptr::null(),
            flags: vk::PipelineRasterizationStateCreateFlags::empty(),
            depth_clamp_enable: vk::FALSE,
            rasterizer_discard_enable: vk::FALSE,
            polygon_mode: vk::PolygonMode::FILL,
            line_width: 1.0,
            cull_mode: vk::CullModeFlags::NONE,
            front_face: vk::FrontFace::CLOCKWISE,
            depth_bias_enable: vk::FALSE,
            depth_bias_constant_factor: 0.0,
            depth_bias_clamp: 0.0,
            depth_bias_slope_factor: 0.0,
            ..Default::default()
        };

        // Multisampling
        let multisample_info = PipelineMultisampleStateCreateInfoOwned {
            s_type: vk::StructureType::PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            p_next: std::ptr::null(),
            flags: vk::PipelineMultisampleStateCreateFlags::empty(),
            rasterization_samples: vk::SampleCountFlags::TYPE_1,
            sample_shading_enable: vk::FALSE,
            min_sample_shading: 1.0,
            p_sample_mask: std::ptr::null(),
            alpha_to_coverage_enable: vk::FALSE,
            alpha_to_one_enable: vk::FALSE,
            ..Default::default()
        };

        // Color blend attachments
        let mut color_blend_attachments = vec![
            vk::PipelineColorBlendAttachmentState {
                color_write_mask: vk::ColorComponentFlags::R
                    | vk::ColorComponentFlags::G
                    | vk::ColorComponentFlags::B
                    | vk::ColorComponentFlags::A,
                blend_enable: if alpha_blending { vk::TRUE } else { vk::FALSE },
                src_color_blend_factor: if alpha_blending {
                    vk::BlendFactor::SRC_ALPHA
                } else {
                    vk::BlendFactor::ONE
                },
                dst_color_blend_factor: if alpha_blending {
                    vk::BlendFactor::ONE_MINUS_SRC_ALPHA
                } else {
                    vk::BlendFactor::ZERO
                },
                color_blend_op: vk::BlendOp::ADD,
                src_alpha_blend_factor: vk::BlendFactor::ONE,
                dst_alpha_blend_factor: vk::BlendFactor::ZERO,
                alpha_blend_op: vk::BlendOp::ADD,
            };
            attachment_count as usize
        ];

        let color_blend_info = PipelineColorBlendStateCreateInfoOwned {
            s_type: vk::StructureType::PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            p_next: std::ptr::null(),
            flags: vk::PipelineColorBlendStateCreateFlags::empty(),
            logic_op_enable: vk::FALSE,
            logic_op: vk::LogicOp::COPY,
            attachment_count,
            p_attachments: color_blend_attachments.as_ptr(),
            blend_constants: [0.0; 4],
            ..Default::default()
        };

        // Depth stencil
        let depth_stencil_info = PipelineDepthStencilStateCreateInfoOwned {
            s_type: vk::StructureType::PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            p_next: std::ptr::null(),
            flags: vk::PipelineDepthStencilStateCreateFlags::empty(),
            depth_test_enable: vk::TRUE,
            depth_write_enable: vk::TRUE,
            depth_compare_op: vk::CompareOp::LESS,
            depth_bounds_test_enable: vk::FALSE,
            stencil_test_enable: vk::FALSE,
            front: vk::StencilOpState::default(),
            back: vk::StencilOpState::default(),
            min_depth_bounds: 0.0,
            max_depth_bounds: 1.0,
            ..Default::default()
        };

        VlGraphicsPipelineConfig {
            viewport,
            scissor,
            input_assembly_info,
            rasterization_info,
            multisample_info,
            color_blend_attachments,
            color_blend_info,
            depth_stencil_info,
            subpass: 0,
        }
    }
}
