use ash::vk;
use crate::pipelines::graphics_pipeline::config::VlGraphicsPipelineConfig;
use crate::pipelines::pipeline_config::pipeline_builder::{VlPipelineBuilder, VlPushConstantInfo, VlSamplerInfo, VlStorageBufferInfo, VlStorageImageInfo, VlUniformBufferInfo, VlVertexInput};

pub struct VlGraphicsPipelineBuilder{
    builder: VlPipelineBuilder,
    color_attachments: Vec<vk::Format>,
    depth_attachment_info: Option<vk::Format>,
    pipeline_info_strip: Option<VlGraphicsPipelineConfig>,
    attachments_per_step_amount: u32,
    width: u32,
    height: u32,
    alpha_blending: bool,
}

impl VlGraphicsPipelineBuilder{
    pub fn new(attachments_per_step_amount: u32, width: u32, height: u32, alpha_blending: bool) -> Self{
        Self{
            builder: VlPipelineBuilder::default(),
            color_attachments: Vec::new(),
            depth_attachment_info: None,
            pipeline_info_strip: None,
            attachments_per_step_amount,
            width,
            height,
            alpha_blending,
        }
    }

    pub fn add_color_attachment(&mut self, color_format: vk::Format) {
        self.color_attachments.push(color_format);
    }

    pub fn set_depth_attachment(&mut self, depth_format: vk::Format) {
        self.depth_attachment_info = Some(depth_format);
    }

    pub fn add_vertex_input(&mut self, input: VlVertexInput){
        self.builder.add_vertex_input(input)
    }

    pub fn add_storage_buffer(&mut self, info: VlStorageBufferInfo){
        self.builder.add_storage_buffer(info);
    }

    pub fn add_push_constant(&mut self, info: VlPushConstantInfo){
        self.builder.add_push_constant_info(info);
    }
    pub fn add_uniform_buffer(&mut self, info: VlUniformBufferInfo){
        self.builder.add_uniform_buffer(info);
    }

    pub fn add_sample_info(&mut self, info: VlSamplerInfo){
        self.builder.add_sampler_info(info);
    }

    pub fn add_storage_image_info(&mut self, info: VlStorageImageInfo){
        self.builder.add_storage_image_info(info);
    }

    pub fn get_pipeline_info_strip(&mut self) -> &mut VlGraphicsPipelineConfig{
        if self.pipeline_info_strip.is_none() {
            self.pipeline_info_strip = Some(VlGraphicsPipelineConfig::create_config(self.attachments_per_step_amount, self.alpha_blending, self.width, self.height));
        }
        self.pipeline_info_strip.as_mut().unwrap()
    }

    pub fn builder(&self) -> &VlPipelineBuilder {
        &self.builder
    }

    pub fn color_attachments(&self) -> &Vec<vk::Format> {
        &self.color_attachments
    }

    pub fn depth_attachment_info(&self) -> Option<vk::Format> {
        self.depth_attachment_info
    }
    
    pub fn attachments_per_step_amount(&self) -> u32 {
        self.attachments_per_step_amount
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn alpha_blending(&self) -> bool {
        self.alpha_blending
    }
}