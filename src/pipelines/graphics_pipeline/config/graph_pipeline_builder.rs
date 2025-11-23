use ash::vk;
use crate::pipelines::pipeline_config::pipeline_builder::{VlPipelineBuilder, VlSamplerInfo, VlStorageBufferInfo, VlStorageImageInfo, VlUniformBufferInfo, VlVertexInput};

pub struct VlGraphicsPipelineBuilder{
    builder: VlPipelineBuilder,
    color_attachments: Vec<vk::Format>,
    depth_attachment_info: Option<vk::Format>,
}

impl VlGraphicsPipelineBuilder{
    pub fn new() -> Self{
        Self{
            builder: VlPipelineBuilder::default(),
            color_attachments: Vec::new(),
            depth_attachment_info: None,
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

    pub fn add_uniform_buffer(&mut self, info: VlUniformBufferInfo){
        self.builder.add_uniform_buffer(info);
    }

    pub fn add_sample_info(&mut self, info: VlSamplerInfo){
        self.builder.add_sampler_info(info);
    }

    pub fn add_storage_image_info(&mut self, info: VlStorageImageInfo){
        self.builder.add_storage_image_info(info);
    }
}