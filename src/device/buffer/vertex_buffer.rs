use crate::device::buffer::buffer::VlBuffer;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;
use ash::vk::DeviceSize;
use std::ffi::c_void;
use std::ptr::copy;

pub struct VlVertexBuffer {
    buffer: VlBuffer,
    format: vk::Format,
    step_size: usize,
    vertices_amount: usize,
    vertex_count: usize,
    device: ash::Device,
}

impl VlVertexBuffer {
    pub fn new(
        device: &VlLogicalDevice,
        vertices: *const c_void,
        vertices_amount: usize,
        step_size: usize,
        format: vk::Format,
        for_rt: bool,
    ) -> Self {
        let mut create_info = vk::BufferCreateInfo::default()
            .size((vertices_amount * step_size) as DeviceSize)
            .usage(vk::BufferUsageFlags::TRANSFER_SRC)
            .sharing_mode(vk::SharingMode::EXCLUSIVE);
        let stg_buffer = VlBuffer::new(
            device,
            &create_info,
            vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT,
            false,
        );
        let m_point = stg_buffer.map_buffer(0, vk::MemoryMapFlags::empty());
        unsafe {
            copy(vertices, m_point, vertices_amount * step_size);
        }
        stg_buffer.un_map();

        create_info.usage = vk::BufferUsageFlags::VERTEX_BUFFER
            | vk::BufferUsageFlags::TRANSFER_DST
            | if for_rt {
                vk::BufferUsageFlags::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_KHR
                    | vk::BufferUsageFlags::SHADER_DEVICE_ADDRESS
            } else {
                vk::BufferUsageFlags::empty()
            };
        let dst_buffer = VlBuffer::new(device, &create_info, vk::MemoryPropertyFlags::DEVICE_LOCAL, for_rt);
        let cmd = device.find_queue_by_type_r(vk::QueueFlags::GRAPHICS).unwrap().begin_single_time_command();
        dst_buffer.copy_from_buffer(cmd, &stg_buffer, create_info.size, 0, 0);
        device.find_queue_by_type_r(vk::QueueFlags::GRAPHICS).unwrap().end_single_time_command(cmd);
        drop(stg_buffer);
        Self{
            buffer: dst_buffer,
            format,
            step_size,
            vertices_amount,
            vertex_count: vertices_amount,
            device: device.device(),
        }
    }
    
    pub fn bind(&self, cmd: vk::CommandBuffer) {
        let buffers = [self.buffer.buffer()];
        let offsets = [vk::DeviceSize::default()];
        unsafe { self.device.cmd_bind_vertex_buffers(cmd, 0, &buffers, &offsets); }
    }
    
    pub fn draw_all(&self, cmd: vk::CommandBuffer) {
        unsafe { self.device.cmd_draw(cmd, self.vertex_count as u32, 1, 0, 0); }
    }
}
