use crate::device::descriptors::descriptors_infos::{
    VlDescriptorAccelerationStructureInfo, VlDescriptorBufferInfo, VlDescriptorImageInfo,
};
use ash::vk;
use ash::vk::WriteDescriptorSetAccelerationStructureKHR;

pub struct VlDescriptorPool {
    descriptor_pool: vk::DescriptorPool,
    device: ash::Device,
}

pub struct VlDescriptorSet {
    descriptor_sets: Vec<vk::DescriptorSet>,
    device: ash::Device,
    buffers_infos: Vec<VlDescriptorBufferInfo>,
    image_infos: Vec<VlDescriptorImageInfo>,
    as_infos: Vec<VlDescriptorAccelerationStructureInfo>,
    image_info_per_instance_amount: u32,
    buffer_info_per_instance_amount: u32,
}

impl VlDescriptorPool {
    pub fn new(device: ash::Device, support_as: bool) -> Self {
        let mut pool_sizes = Vec::with_capacity(10 + (support_as as usize));
        for i in 0..10 {
            pool_sizes.push(vk::DescriptorPoolSize {
                ty: vk::DescriptorType::from_raw(i),
                descriptor_count: 10000,
            });
        }
        if support_as {
            pool_sizes.push(vk::DescriptorPoolSize {
                ty: vk::DescriptorType::ACCELERATION_STRUCTURE_KHR,
                descriptor_count: 10000,
            });
        }
        let pool_info = vk::DescriptorPoolCreateInfo::default()
            .max_sets(1500)
            .pool_sizes(pool_sizes.as_slice());
        let pool = unsafe { device.create_descriptor_pool(&pool_info, None) }
            .expect("Failed to create descriptor pool");
        Self {
            descriptor_pool: pool,
            device,
        }
    }

    pub fn allocate_descriptor_set(
        &self,
        instance_count: u32,
        layout: vk::DescriptorSetLayout,
    ) -> VlDescriptorSet {
        let layouts = vec![layout; instance_count as usize];
        let mut allocate_info = vk::DescriptorSetAllocateInfo::default()
            .descriptor_pool(self.descriptor_pool)
            .set_layouts(layouts.as_slice());
        allocate_info.descriptor_set_count = instance_count;
        let res = unsafe { self.device.allocate_descriptor_sets(&allocate_info) }
            .expect("Failed to allocate descriptor set");
        VlDescriptorSet {
            descriptor_sets: res,
            device: self.device.clone(),
            buffers_infos: Vec::new(),
            image_infos: Vec::new(),
            as_infos: Vec::new(),
            image_info_per_instance_amount: 0,
            buffer_info_per_instance_amount: 0,
        }
    }

    pub fn destroy(&mut self) {
        unsafe {
            self.device
                .destroy_descriptor_pool(self.descriptor_pool, None);
        }
    }
}

impl Drop for VlDescriptorPool {
    fn drop(&mut self) {
        self.destroy();
    }
}

impl VlDescriptorSet {
    pub fn update_descriptors(&self) {
        let (mut writes, mut as_infos) = self.make_writes();
        Self::update_as_infos(&mut writes, &mut as_infos);
        let mut counter: usize = 0;
        for item in self.descriptor_sets.iter() {
            let add: usize = (self.image_info_per_instance_amount
                + self.buffer_info_per_instance_amount) as usize;
            for c_item in writes.iter_mut() {
                c_item.dst_set = item.clone();
            }
            let descriptor_copies = vec![];
            if self.image_info_per_instance_amount == 0 && self.buffer_info_per_instance_amount == 0
            {
                unsafe {
                    self.device
                        .update_descriptor_sets(writes.as_slice(), descriptor_copies.as_slice());
                }
            } else {
                unsafe {
                    self.device.update_descriptor_sets(
                        &writes.as_slice()[counter..(counter + add)],
                        descriptor_copies.as_slice(),
                    );
                }
            }
            counter += add;
        }
    }

    pub fn bind_descriptor(
        &self,
        bind_point: vk::PipelineBindPoint,
        current_instance: u32,
        cmd: vk::CommandBuffer,
        layout: vk::PipelineLayout,
    ) {
        let descriptor_sets = [self.descriptor_sets[current_instance as usize]];
        let dyn_offsets = vec![];
        unsafe {
            self.device.cmd_bind_descriptor_sets(
                cmd,
                bind_point,
                layout,
                0,
                &descriptor_sets,
                dyn_offsets.as_slice(),
            )
        }
    }

    pub fn clear_info(&mut self) {
        self.image_infos.clear();
        self.buffers_infos.clear();
        self.as_infos.clear();
    }

    fn make_writes(
        &self,
    ) -> (
        Vec<vk::WriteDescriptorSet<'_>>,
        Vec<vk::WriteDescriptorSetAccelerationStructureKHR<'_>>,
    ) {
        let mut res = Vec::with_capacity(
            self.buffers_infos.len() + self.image_infos.len() + self.as_infos.len(),
        );
        let mut as_infos = Vec::with_capacity(self.as_infos.len());
        self.buffers_infos.iter().for_each(|info| {
            let write_info = vk::WriteDescriptorSet::default()
                .dst_set(self.descriptor_sets[0])
                .dst_binding(info.binding)
                .dst_array_element(0)
                .descriptor_type(info.descriptor_type)
                .buffer_info(info.buffer_infos.as_slice())
                .descriptor_count(info.buffer_infos.len() as u32);
            res.push(write_info);
        });
        self.image_infos.iter().for_each(|info| {
            let write_info = vk::WriteDescriptorSet::default()
                .dst_set(self.descriptor_sets[0])
                .dst_binding(info.binding)
                .dst_array_element(0)
                .descriptor_type(info.descriptor_type)
                .image_info(info.image_infos.as_slice())
                .descriptor_count(info.image_infos.len() as u32);
            res.push(write_info);
        });
        self.as_infos.iter().for_each(|info| {
            as_infos.push(info.base.to_base());
            let write_info = vk::WriteDescriptorSet::default()
                .dst_set(self.descriptor_sets[0])
                .dst_binding(info.binding)
                .dst_array_element(0)
                .descriptor_type(info.descriptor_type)
                .descriptor_count(1);
            res.push(write_info);
        });
        (res, as_infos)
    }

    pub fn update_as_infos<'a>(
        write_infos: &mut Vec<vk::WriteDescriptorSet<'a>>,
        as_infos: &'a mut Vec<WriteDescriptorSetAccelerationStructureKHR<'a>>,
    ) {
        if !as_infos.is_empty(){
            let start = write_infos.len() - as_infos.len() - 1;
            let mut as_infos_iter = as_infos.iter_mut();
            for i in start..write_infos.len() {
                write_infos[i] = write_infos[i].push_next(as_infos_iter.next().unwrap());
            }
        }
    }

    pub fn add_buffer_info(&mut self, info: VlDescriptorBufferInfo) {
        self.buffers_infos.push(info);
    }
    pub fn add_image_info(&mut self, info: VlDescriptorImageInfo) {
        self.image_infos.push(info);
    }
    pub fn add_as_info(&mut self, info: VlDescriptorAccelerationStructureInfo) {
        self.as_infos.push(info);
    }

    pub fn set_image_info_per_instance_amount(&mut self, image_info_per_instance_amount: u32) {
        self.image_info_per_instance_amount = image_info_per_instance_amount;
    }

    pub fn set_buffer_info_per_instance_amount(&mut self, buffer_info_per_instance_amount: u32) {
        self.buffer_info_per_instance_amount = buffer_info_per_instance_amount;
    }

    pub fn descriptor_sets(&self) -> &Vec<vk::DescriptorSet> {
        &self.descriptor_sets
    }
}
