use ash::vk;
use ash::Device;
use crate::pipelines::pipeline_config::pipeline_builder::*;

pub struct VlPipelineConfigurer {
    device: Device,
    pipeline_layout: vk::PipelineLayout,
    descriptor_set_layout: Option<vk::DescriptorSetLayout>,
    input_bind_desc: vk::VertexInputBindingDescription,
    input_attrib_descs: Vec<vk::VertexInputAttributeDescription>,
}

impl Drop for VlPipelineConfigurer {
    fn drop(&mut self) {
        unsafe { self.destroy() }
    }
}

impl VlPipelineConfigurer {
    pub fn new(device: Device, builder: &VlPipelineBuilder) -> Self {
        let mut configurer = VlPipelineConfigurer {
            device,
            pipeline_layout: vk::PipelineLayout::null(),
            descriptor_set_layout: None,
            input_bind_desc: vk::VertexInputBindingDescription::default(),
            input_attrib_descs: Vec::new(),
        };

        configurer.load_descriptor_set_layout(builder);
        configurer.load_pipeline_layout(builder);
        configurer.prepare_binding(&builder.vertex_inputs);
        configurer.prepare_input_attribs(&builder.vertex_inputs);

        configurer
    }



    fn load_descriptor_set_layout(&mut self, builder: &VlPipelineBuilder) {
        let total_bindings = builder.uniform_buffer_info.len()
            + builder.samplers_info.len()
            + builder.storage_images_info.len()
            + builder.acceleration_structures_info.len()
            + builder.storage_buffer_infos.len();

        let mut bindings = vec![vk::DescriptorSetLayoutBinding::default(); total_bindings];
        let mut counter = 0;

        for element in &builder.uniform_buffer_info {
            Self::ubo_to_bind(element, &mut bindings[counter]);
            counter += 1;
        }
        for element in &builder.samplers_info {
            Self::sampler_to_bind(element, &mut bindings[counter]);
            counter += 1;
        }
        for item in &builder.acceleration_structures_info {
            Self::as_to_bind(item, &mut bindings[counter]);
            counter += 1;
        }
        for item in &builder.storage_images_info {
            Self::storage_image_to_bind(item, &mut bindings[counter]);
            counter += 1;
        }
        for item in &builder.storage_buffer_infos {
            Self::sbo_to_bind(item, &mut bindings[counter]);
            counter += 1;
        }

        if !bindings.is_empty() {
            let layout_info = vk::DescriptorSetLayoutCreateInfo::default()
                .bindings(&bindings);

            self.descriptor_set_layout = Some(
                unsafe {
                    self.device
                        .create_descriptor_set_layout(&layout_info, None)
                        .expect("Failed to create descriptor set layout")
                }
            );
        }
    }

    fn load_pipeline_layout(&mut self, builder: &VlPipelineBuilder) {
        let mut push_constant_ranges = vec![vk::PushConstantRange::default(); builder.push_constant_infos.len()];
        for (i, element) in builder.push_constant_infos.iter().enumerate() {
            Self::info_to_range(element, &mut push_constant_ranges[i]);
        }

        let layout_info = vk::PipelineLayoutCreateInfo::default()
            .set_layouts(self.descriptor_set_layout.as_ref().map_or(&[][..], |dsl| std::slice::from_ref(dsl)))
            .push_constant_ranges(&push_constant_ranges);

        self.pipeline_layout = unsafe {
            self.device
                .create_pipeline_layout(&layout_info, None)
                .expect("Failed to create pipeline layout")
        };
    }

    fn prepare_binding(&mut self, inputs: &[VlVertexInput]) {
        let stride: usize = inputs.iter().map(|i| i.type_size * i.coordinates_amount as usize).sum();
        self.input_bind_desc.binding = 0;
        self.input_bind_desc.stride = stride as u32;
        self.input_bind_desc.input_rate = vk::VertexInputRate::VERTEX;
    }

    fn prepare_input_attribs(&mut self, inputs: &[VlVertexInput]) {
        self.input_attrib_descs.resize(inputs.len(), vk::VertexInputAttributeDescription::default());
        let mut offset_count = 0;

        for (i, element) in inputs.iter().enumerate() {
            self.input_attrib_descs[i].binding = 0;
            self.input_attrib_descs[i].location = element.location;
            self.input_attrib_descs[i].offset = offset_count as u32;
            self.input_attrib_descs[i].format = element.format;
            offset_count += element.type_size * element.coordinates_amount as usize;
        }
    }

    fn info_to_range(info: &VlPushConstantInfo, range: &mut vk::PushConstantRange) {
        range.stage_flags = info.shader_stages;
        range.offset = 0;
        range.size = info.size as u32;
    }

    fn ubo_to_bind(info: &VlUniformBufferInfo, binding: &mut vk::DescriptorSetLayoutBinding) {
        binding.binding = info.binding;
        binding.descriptor_type = vk::DescriptorType::UNIFORM_BUFFER;
        binding.descriptor_count = info.descriptor_count;
        binding.stage_flags = info.shader_stages;
    }

    fn sbo_to_bind(info: &VlStorageBufferInfo, binding: &mut vk::DescriptorSetLayoutBinding) {
        binding.binding = info.binding;
        binding.descriptor_type = vk::DescriptorType::STORAGE_BUFFER;
        binding.descriptor_count = info.descriptor_count;
        binding.stage_flags = info.stage_flags;
    }

    fn sampler_to_bind(info: &VlSamplerInfo, binding: &mut vk::DescriptorSetLayoutBinding) {
        binding.binding = info.binding;
        binding.descriptor_type = vk::DescriptorType::COMBINED_IMAGE_SAMPLER;
        binding.descriptor_count = info.descriptor_count;
        binding.stage_flags = info.shader_stages;
    }

    fn as_to_bind(info: &VlAccelerationStructureInfo, binding: &mut vk::DescriptorSetLayoutBinding) {
        binding.binding = info.binding;
        binding.descriptor_type = vk::DescriptorType::ACCELERATION_STRUCTURE_KHR;
        binding.descriptor_count = info.descriptor_count;
        binding.stage_flags = info.shader_stages;
    }

    fn storage_image_to_bind(info: &VlStorageImageInfo, binding: &mut vk::DescriptorSetLayoutBinding) {
        binding.binding = info.binding;
        binding.descriptor_type = vk::DescriptorType::STORAGE_IMAGE;
        binding.descriptor_count = info.descriptor_count;
        binding.stage_flags = info.shader_stages;
    }

    pub unsafe fn destroy(&mut self) {
        if self.pipeline_layout != vk::PipelineLayout::null() {
            self.device.destroy_pipeline_layout(self.pipeline_layout, None);
        }
        if let Some(layout) = self.descriptor_set_layout.take() {
            self.device.destroy_descriptor_set_layout(layout, None);
        }
    }

    pub fn pipeline_layout(&self) -> vk::PipelineLayout {
        self.pipeline_layout
    }

    pub fn descriptor_set_layout(&self) -> Option<vk::DescriptorSetLayout> {
        self.descriptor_set_layout
    }

    pub fn input_bind_desc(&self) -> vk::VertexInputBindingDescription {
        self.input_bind_desc
    }

    pub fn input_attrib_descs(&self) -> &Vec<vk::VertexInputAttributeDescription> {
        &self.input_attrib_descs
    }

}
