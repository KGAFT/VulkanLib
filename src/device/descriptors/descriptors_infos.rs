use std::ffi::c_void;
use ash::vk;
use ash::vk::{AccelerationStructureKHR, StructureType};

pub struct VlDescriptorBufferInfo{
    pub buffer_infos: Vec<vk::DescriptorBufferInfo>,
    pub binding: u32,
    pub descriptor_type: vk::DescriptorType
}
pub struct VlDescriptorImageInfo{
    pub image_infos: Vec<vk::DescriptorImageInfo>,
    pub binding: u32,
    pub descriptor_type: vk::DescriptorType
}
#[derive(Default)]
pub struct WriteDescriptorSetAccelerationStructureKHROwned{
    pub s_type: StructureType,
    pub p_next: *const c_void,
    pub acceleration_structure_count: u32,
    pub p_acceleration_structures: *const AccelerationStructureKHR,
}

impl WriteDescriptorSetAccelerationStructureKHROwned{
    pub fn from_base(base: &vk::WriteDescriptorSetAccelerationStructureKHR) -> Self{
        Self{
            s_type: base.s_type,
            p_next: base.p_next,
            acceleration_structure_count: base.acceleration_structure_count,
            p_acceleration_structures: base.p_acceleration_structures
        }
    }
    pub fn from_array_base(base: &[vk::WriteDescriptorSetAccelerationStructureKHR]) -> Vec<Self>{
        let mut res = Vec::with_capacity(base.len());
        base.iter().for_each(|el| res.push(Self::from_base(el)));
        res
    }

    pub fn to_base(&self) -> vk::WriteDescriptorSetAccelerationStructureKHR<'_>{
        vk::WriteDescriptorSetAccelerationStructureKHR{
            s_type: self.s_type,
            p_next: self.p_next,
            acceleration_structure_count: self.acceleration_structure_count,
            p_acceleration_structures: self.p_acceleration_structures,
            ..Default::default()
        }
    }
}

pub struct VlDescriptorAccelerationStructureInfo{
    pub base: WriteDescriptorSetAccelerationStructureKHROwned,
    pub binding: u32,
    pub descriptor_type: vk::DescriptorType
}