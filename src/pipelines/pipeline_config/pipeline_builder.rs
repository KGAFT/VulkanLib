use ash::vk;

#[derive(Clone, Copy, Debug)]
pub struct VertexInput {
    pub location: u32,
    pub coordinates_amount: u32,
    pub type_size: usize,
    pub format: vk::Format,
}

#[derive(Clone, Copy, Debug)]
pub struct PushConstantInfo {
    pub shader_stages: vk::ShaderStageFlags,
    pub size: usize,
}

#[derive(Clone, Copy, Debug)]
pub struct UniformBufferInfo {
    pub binding: u32,
    pub size: usize,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct SamplerInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct AccelerationStructureInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct StorageImageInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub shader_stages: vk::ShaderStageFlags,
}

#[derive(Clone, Copy, Debug)]
pub struct StorageBufferInfo {
    pub binding: u32,
    pub descriptor_count: u32,
    pub stage_flags: vk::ShaderStageFlags,
}

#[derive(Default)]
pub struct PipelineBuilder {
    pub vertex_inputs: Vec<VertexInput>,
    pub uniform_buffer_info: Vec<UniformBufferInfo>,
    pub push_constant_infos: Vec<PushConstantInfo>,
    pub samplers_info: Vec<SamplerInfo>,
    pub acceleration_structures_info: Vec<AccelerationStructureInfo>,
    pub storage_images_info: Vec<StorageImageInfo>,
    pub storage_buffer_infos: Vec<StorageBufferInfo>,
}

impl PipelineBuilder {
    pub fn add_vertex_input(&mut self, input: VertexInput) {
        self.vertex_inputs.push(input);
    }

    pub fn add_push_constant_info(&mut self, info: PushConstantInfo) {
        self.push_constant_infos.push(info);
    }

    pub fn add_uniform_buffer(&mut self, info: UniformBufferInfo) {
        self.uniform_buffer_info.push(info);
    }

    pub fn add_storage_buffer(&mut self, info: StorageBufferInfo) {
        self.storage_buffer_infos.push(info);
    }

    pub fn add_sampler_info(&mut self, info: SamplerInfo) {
        self.samplers_info.push(info);
    }

    pub fn add_acceleration_structure_info(&mut self, info: AccelerationStructureInfo) {
        self.acceleration_structures_info.push(info);
    }

    pub fn add_storage_image_info(&mut self, info: StorageImageInfo) {
        self.storage_images_info.push(info);
    }
}
