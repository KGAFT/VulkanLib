use crate::device::buffer::buffer::VlBuffer;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;
use ash::vk::DeviceSize;
use std::ffi::c_void;

pub struct VlIndirectBuffer {
    buffer: VlBuffer,
    structures_amount: u32,
    size_of_structure: usize,
    mapped_ptr: *mut c_void,
}

impl VlIndirectBuffer {
    pub fn new(
        device: &VlLogicalDevice,
        structures_amount: u32,
        size_of_structure: usize,
    ) -> Self {
        let buffer_size =
            (structures_amount as usize * size_of_structure) as DeviceSize;

        let create_info = vk::BufferCreateInfo::default()
            .size(buffer_size)
            .usage(
                vk::BufferUsageFlags::INDIRECT_BUFFER
                    | vk::BufferUsageFlags::TRANSFER_DST
                    | vk::BufferUsageFlags::TRANSFER_SRC,
            )
            .sharing_mode(vk::SharingMode::EXCLUSIVE);

        let buffer = VlBuffer::new(
            device,
            &create_info,
            vk::MemoryPropertyFlags::HOST_VISIBLE
                | vk::MemoryPropertyFlags::HOST_COHERENT,
            false,
        );

        // Persistent mapping
        let mapped_ptr = buffer.map_buffer(0, vk::MemoryMapFlags::empty());

        Self {
            buffer,
            structures_amount,
            size_of_structure,
            mapped_ptr,
        }
    }

    pub fn map_point(&self) -> *mut c_void {
        self.mapped_ptr
    }

    pub fn buffer(&self) -> vk::Buffer {
        self.buffer.buffer()
    }

    pub fn structures_amount(&self) -> u32 {
        self.structures_amount
    }

    pub fn size_of_structure(&self) -> usize {
        self.size_of_structure
    }

}

impl Drop for VlIndirectBuffer {
    fn drop(&mut self) {
        self.buffer.un_map()
    }
}
