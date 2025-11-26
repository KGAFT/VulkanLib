use crate::pipelines::graphics_pipeline::config::graph_pipeline_builder::VlGraphicsPipelineBuilder;
use crate::pipelines::graphics_pipeline::config::VlGraphicsPipelineConfig;
use crate::pipelines::pipeline_config::pipeline_configurer::VlPipelineConfigurer;
use crate::pipelines::shader::VlShader;
use ash::vk;

pub mod config;
#[derive(Default)]
struct GraphicsPipelineCreateStrip<'a> {
    pub vertex_input_info: vk::PipelineVertexInputStateCreateInfo<'a>,
    pub viewport_info: vk::PipelineViewportStateCreateInfo<'a>,
    pub pipeline_info: vk::GraphicsPipelineCreateInfo<'a>,
}

pub struct VlGraphicsPipeline {
    attachments_formats: Vec<vk::Format>,
    depth_format: Option<vk::Format>,
    configurer: VlPipelineConfigurer,
    graphics_pipeline: vk::Pipeline,
    viewport: vk::Viewport,
    scissor: vk::Rect2D,
    device: ash::Device,
    shader: Option<VlShader>,
    attachment_per_step_amount: u32,
}

impl VlGraphicsPipeline {
    pub fn new(device: ash::Device, shader: VlShader, mut builder: VlGraphicsPipelineBuilder) -> Self {
        let mut result = Self {
            attachments_formats: vec![],
            depth_format: None,
            configurer: VlPipelineConfigurer::new(device.clone(), builder.builder()),
            graphics_pipeline: Default::default(),
            viewport: Default::default(),
            scissor: Default::default(),
            device,
            shader: None,
            attachment_per_step_amount: builder.attachments_per_step_amount(),
        };
        unsafe {
            result.create(
                builder.attachments_per_step_amount(),
                shader,
                builder.color_attachments().clone(),
                builder.depth_attachment_info(),
                builder.get_pipeline_info_strip(),
            );
        }
        result
    }
    pub fn get_graphics_pipeline(&self) -> vk::Pipeline {
        self.graphics_pipeline
    }

    pub fn resize(&mut self, width: u32, height: u32) {
        self.viewport.width = width as f32;
        self.viewport.height = height as f32;
        self.scissor.extent.width = width;
        self.scissor.extent.height = height;
    }

    pub fn get_viewport(&mut self) -> &mut vk::Viewport {
        &mut self.viewport
    }

    pub fn get_scissor(&mut self) -> &mut vk::Rect2D {
        &mut self.scissor
    }

    pub fn get_descriptor_layout(&self) -> Option<vk::DescriptorSetLayout> {
        self.configurer.descriptor_set_layout()
    }

    pub fn get_pipeline_layout(&self) -> vk::PipelineLayout {
        self.configurer.pipeline_layout()
    }

    unsafe fn create(
        &mut self,
        attachment_per_step_amount: u32,
        shader: VlShader,
        color_formats: Vec<vk::Format>,
        depth_format: Option<vk::Format>,
        config: &VlGraphicsPipelineConfig,
    ) {
        self.shader = Some(shader);
        self.attachment_per_step_amount = attachment_per_step_amount;
        self.depth_format = depth_format;
        self.attachments_formats = color_formats.clone();
        self.viewport = config.viewport;
        self.scissor = config.scissor;

        let mut create_strip = GraphicsPipelineCreateStrip::default();

        create_strip.vertex_input_info = vk::PipelineVertexInputStateCreateInfo::default()
            .vertex_attribute_descriptions(&self.configurer.input_attrib_descs)
            .vertex_binding_descriptions(std::slice::from_ref(&self.configurer.input_bind_desc));

        create_strip.viewport_info = vk::PipelineViewportStateCreateInfo::default()
            .viewport_count(1)
            .scissor_count(1);

        let shader_stages = self.shader.as_ref().unwrap().create_infos_vk();

        let input_asm_info = config.input_assembly_info.to_base();
        let raster_info = config.rasterization_info.to_base();
        let multisample_info = config.multisample_info.to_base();
        let color_blend_info = config.color_blend_info.to_base();
        let depth_stencil_info = config.depth_stencil_info.to_base();
        create_strip.pipeline_info = vk::GraphicsPipelineCreateInfo::default()
            .stages(shader_stages.as_slice())
            .vertex_input_state(&create_strip.vertex_input_info)
            .input_assembly_state(&input_asm_info)
            .rasterization_state(&raster_info)
            .multisample_state(&multisample_info)
            .color_blend_state(&color_blend_info)
            .depth_stencil_state(&depth_stencil_info)
            .viewport_state(&create_strip.viewport_info)
            .layout(self.configurer.pipeline_layout())
            .subpass(config.subpass)
            .base_pipeline_index(-1);

        let mut rendering_info = vk::PipelineRenderingCreateInfo::default()
            .color_attachment_formats(&color_formats);
        if self.depth_format.is_some(){
            rendering_info = rendering_info.depth_attachment_format(self.depth_format.as_ref().unwrap().clone());
        }

        let dyn_states = [vk::DynamicState::VIEWPORT, vk::DynamicState::SCISSOR];

        let dynamic_state_info =
            vk::PipelineDynamicStateCreateInfo::default().dynamic_states(&dyn_states);

        create_strip.pipeline_info = create_strip.pipeline_info.push_next(&mut rendering_info);
        create_strip.pipeline_info = create_strip.pipeline_info.dynamic_state(&dynamic_state_info);

        let result = self
            .device
            .create_graphics_pipelines(
                vk::PipelineCache::null(),
                &[create_strip.pipeline_info],
                None,
            )
            .unwrap();

        self.graphics_pipeline = result[0];
    }

    pub fn destroy(&mut self) {
        unsafe { self.device.destroy_pipeline(self.graphics_pipeline, None); }
    }
}

impl Drop for VlGraphicsPipeline {
    fn drop(&mut self) {
        self.destroy();
    }
}
