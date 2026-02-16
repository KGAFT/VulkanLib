use crate::device::buffer::buffer::VlBuffer;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;
use ash::vk::DeviceSize;
use std::ffi::c_void;

pub struct VlStorageBuffer {
    buffer: VlBuffer,
    buffer_size: usize,
    mapped_ptr: *mut c_void,
}

impl VlStorageBuffer {
    pub fn new(
        device: &VlLogicalDevice,
        buffer_size: usize,
        additional_flags: vk::BufferUsageFlags,
    ) -> Self {
        let usage = vk::BufferUsageFlags::STORAGE_BUFFER
            | vk::BufferUsageFlags::TRANSFER_DST
            | vk::BufferUsageFlags::TRANSFER_SRC
            | additional_flags;

        let create_info = vk::BufferCreateInfo::default()
            .size(buffer_size as DeviceSize)
            .usage(usage)
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
            buffer_size,
            mapped_ptr,
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

    pub fn base(&self) -> &VlBuffer {
        &self.buffer
    }
}

impl Drop for VlStorageBuffer {
    fn drop(&mut self) {
        self.buffer.un_map();

    }
}