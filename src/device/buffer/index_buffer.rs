use crate::device::buffer::buffer::VlBuffer;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;
use ash::vk::DeviceSize;
use std::ffi::c_void;
use std::ptr::copy;

pub struct VlIndexBuffer {
    buffer: VlBuffer,
    index_count: u32,
    index_type: vk::IndexType,
    device: ash::Device,
}

impl VlIndexBuffer {
    pub fn new(
        device: &VlLogicalDevice,
        indices: *const c_void,
        index_count: u32,
        index_type: vk::IndexType,
        for_rt: bool,
    ) -> Self {
        let buffer_size =
            (index_count as usize * std::mem::size_of::<u32>()) as DeviceSize;

        let mut create_info = vk::BufferCreateInfo::default()
            .size(buffer_size)
            .usage(vk::BufferUsageFlags::TRANSFER_SRC)
            .sharing_mode(vk::SharingMode::EXCLUSIVE);

        let staging_buffer = VlBuffer::new(
            device,
            &create_info,
            vk::MemoryPropertyFlags::HOST_VISIBLE
                | vk::MemoryPropertyFlags::HOST_COHERENT,
            false,
        );

        let mapped_ptr = staging_buffer.map_buffer(0, vk::MemoryMapFlags::empty());

        unsafe {
            copy(indices, mapped_ptr, buffer_size as usize);
        }

        staging_buffer.un_map();

        create_info.usage =
            vk::BufferUsageFlags::INDEX_BUFFER
                | vk::BufferUsageFlags::TRANSFER_DST
                | if for_rt {
                vk::BufferUsageFlags::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_KHR
                    | vk::BufferUsageFlags::SHADER_DEVICE_ADDRESS
            } else {
                vk::BufferUsageFlags::empty()
            };

        let device_buffer = VlBuffer::new(
            device,
            &create_info,
            vk::MemoryPropertyFlags::DEVICE_LOCAL,
            for_rt,
        );

        let queue = device
            .find_queue_by_type_r(vk::QueueFlags::GRAPHICS)
            .unwrap();

        let cmd = queue.begin_single_time_command();

        device_buffer.copy_from_buffer(cmd, &staging_buffer, buffer_size, 0, 0);

        queue.end_single_time_command(cmd);

        drop(staging_buffer);

        Self {
            buffer: device_buffer,
            index_count,
            index_type,
            device: device.device(),
        }
    }

    pub fn bind(&self, cmd: vk::CommandBuffer) {
        unsafe {
            self.device.cmd_bind_index_buffer(
                cmd,
                self.buffer.buffer(),
                0,
                self.index_type,
            );
        }
    }

    pub fn draw_all(&self, cmd: vk::CommandBuffer) {
        unsafe {
            self.device
                .cmd_draw_indexed(cmd, self.index_count, 1, 0, 0, 0);
        }
    }

    pub fn index_count(&self) -> u32 {
        self.index_count
    }

    pub fn index_type(&self) -> vk::IndexType {
        self.index_type
    }
}
