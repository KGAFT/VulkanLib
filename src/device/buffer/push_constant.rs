use ash::vk;

pub struct VlPushConstant {
    data: Vec<u8>,
    layout: vk::PipelineLayout,
}

impl VlPushConstant {
    pub fn new(size: usize, layout: vk::PipelineLayout) -> Self {
        Self {
            data: vec![0u8; size],
            layout,
        }
    }

    pub fn bind(
        &self,
        device: &ash::Device,
        cmd: vk::CommandBuffer,
        stage_flags: vk::ShaderStageFlags,
    ) {
        unsafe {
            device.cmd_push_constants(
                cmd,
                self.layout,
                stage_flags,
                0,
                &self.data,
            );
        }
    }

    pub fn write_data(&mut self, src: &[u8]) {
        if src.len() > self.data.len() {
            panic!("Push constant write exceeds allocated size");
        }

        self.data[..src.len()].copy_from_slice(src);
    }

    pub fn size(&self) -> usize {
        self.data.len()
    }
}
