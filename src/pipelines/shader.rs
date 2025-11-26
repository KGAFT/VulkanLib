use ash::vk;
use ash::Device;
use crate::pipelines::shader_stage_create_info_owned::PipelineShaderStageCreateInfoOwned;

pub struct VlShader {
    create_infos: Vec<PipelineShaderStageCreateInfoOwned>,
    device: ash::Device,
    rt_sorted: bool,
    destroyed: bool,
}

impl VlShader {
    pub fn new(
        mut shader_modules: Vec<vk::PipelineShaderStageCreateInfo>,
        device: ash::Device,
    ) -> Self {
        let mut new_infos = Vec::with_capacity(shader_modules.len());
        while let Some(info) = shader_modules.pop() {
            unsafe { new_infos.push(PipelineShaderStageCreateInfoOwned::from_vulkan(&info)); }
        }
        Self {
            create_infos: new_infos,
            device,
            rt_sorted: false,
            destroyed: false,
        }
    }

    pub fn create_infos(&mut self) -> &mut Vec<PipelineShaderStageCreateInfoOwned> {
        &mut self.create_infos
    }
    
    pub fn create_infos_vk(&self) -> Vec<vk::PipelineShaderStageCreateInfo>{
        let mut result = Vec::with_capacity(self.create_infos.len());
        self.create_infos.iter().for_each(|info|{
            result.push(info.as_vulkan())
        });
        result
    }

    pub fn is_sorted_for_ray_tracing(&self) -> bool {
        self.rt_sorted
    }

    /// Sort RT shaders into RayGen → Hit → Miss → Intersection order
    pub fn sort_ray_tracing_shaders_by_groups(&mut self) {
        let mut raygen = Vec::new();
        let mut hit = Vec::new();
        let mut miss = Vec::new();
        let mut intersection = Vec::new();

        while let Some(info) = self.create_infos.pop() {
            match info.stage {
                vk::ShaderStageFlags::RAYGEN_KHR => raygen.push(info),
                vk::ShaderStageFlags::CLOSEST_HIT_KHR => hit.push(info),
                vk::ShaderStageFlags::ANY_HIT_KHR => hit.push(info),
                vk::ShaderStageFlags::MISS_KHR => miss.push(info),
                vk::ShaderStageFlags::INTERSECTION_KHR => intersection.push(info),
                _ => {}
            }
        }

        let mut new_stages = Vec::new();
        new_stages.extend(raygen);
        new_stages.extend(hit);
        new_stages.extend(miss);
        new_stages.extend(intersection);

        self.create_infos = new_stages;
        self.rt_sorted = true;
    }

    pub fn destroy(&mut self) {
        if self.destroyed {
            return;
        }
        self.destroyed = true;

        unsafe {
            for info in &self.create_infos {
                self.device.destroy_shader_module(info.module, None);
            }
        }
    }
}

impl Drop for VlShader {
    fn drop(&mut self) {
        self.destroy();
    }
}
