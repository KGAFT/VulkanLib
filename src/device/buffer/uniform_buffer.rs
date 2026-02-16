use crate::device::buffer::buffer::VlBuffer;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;
use ash::vk::DeviceSize;
use std::ffi::c_void;

pub struct VlUniformBuffer {
    buffer: VlBuffer,
    mapped_ptr: *mut c_void,
    buffer_size: usize,
}

impl VlUniformBuffer {
    pub fn new(
        device: &VlLogicalDevice,
        buffer_size: usize,
    ) -> Self {
        let create_info = vk::BufferCreateInfo::default()
            .size(buffer_size as DeviceSize)
            .usage(vk::BufferUsageFlags::UNIFORM_BUFFER)
            .sharing_mode(vk::SharingMode::EXCLUSIVE);

        let buffer = VlBuffer::new(
            device,
            &create_info,
            vk::MemoryPropertyFlags::HOST_VISIBLE
                | vk::MemoryPropertyFlags::HOST_COHERENT,
            false,
        );

        let mapped_ptr = buffer.map_buffer(0, vk::MemoryMapFlags::empty());

        Self {
            buffer,
            mapped_ptr,
            buffer_size,
        }
    }

    pub fn map_point(&self) -> *mut c_void {
        self.mapped_ptr
    }

    pub fn buffer(&self) -> vk::Buffer {
        self.buffer.buffer()
    }

    pub fn buffer_size(&self) -> usize {
        self.buffer_size
    }
}

impl Drop for VlUniformBuffer {
    fn drop(&mut self) {
        self.buffer.un_map();
    }
}
