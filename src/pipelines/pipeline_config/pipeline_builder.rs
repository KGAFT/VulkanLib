use ash::vk;

#[derive(Clone, Copy, Debug)]
pub struct VlVertexInput {
    pub location: u32,
    pub coordinates_amount: u32,
    pub type_size: usize,
    pub format: vk::Format,
}

#[derive(Clone, Copy, Debug)]
pub struct VlPushConstantInfo {
    pub shader_stages: vk::ShaderStageFlags,
    pub size: usize,
}

#[derive(Clone, Copy, Debug)]
pub struct VlUniformBufferInfo {
    pub binding: u32,
    pub size: usize,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct VlSamplerInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct VlAccelerationStructureInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct VlStorageImageInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct VlStorageBufferInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub stage_flags: vk::ShaderStageFlags,
}

#[derive(Default)]
pub struct VlPipelineBuilder {
    pub vertex_inputs: Vec<VlVertexInput>,
    pub uniform_buffer_info: Vec<VlUniformBufferInfo>,
    pub push_constant_infos: Vec<VlPushConstantInfo>,
    pub samplers_info: Vec<VlSamplerInfo>,
    pub acceleration_structures_info: Vec<VlAccelerationStructureInfo>,
    pub storage_images_info: Vec<VlStorageImageInfo>,
    pub storage_buffer_infos: Vec<VlStorageBufferInfo>,
}

impl VlPipelineBuilder {
    pub fn add_vertex_input(&mut self, input: VlVertexInput) {
        self.vertex_inputs.push(input);
    }

    pub fn add_push_constant_info(&mut self, info: VlPushConstantInfo) {
        self.push_constant_infos.push(info);
    }

    pub fn add_uniform_buffer(&mut self, info: VlUniformBufferInfo) {
        self.uniform_buffer_info.push(info);
    }

    pub fn add_storage_buffer(&mut self, info: VlStorageBufferInfo) {
        self.storage_buffer_infos.push(info);
    }

    pub fn add_sampler_info(&mut self, info: VlSamplerInfo) {
        self.samplers_info.push(info);
    }

    pub fn add_acceleration_structure_info(&mut self, info: VlAccelerationStructureInfo) {
        self.acceleration_structures_info.push(info);
    }

    pub fn add_storage_image_info(&mut self, info: VlStorageImageInfo) {
        self.storage_images_info.push(info);
    }
}
