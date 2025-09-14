use crate::device::logical_device::logical_device::VlLogicalDevice;
use ash::vk;
use ash::vk::DeviceSize;
use std::ffi::c_void;

pub struct VlBuffer {
    buffer: vk::Buffer,
    memory: vk::DeviceMemory,
    device: ash::Device,
    size: usize,
    origin: bool,
}

impl Drop for VlBuffer {
    fn drop(&mut self) {
        if self.origin {
            unsafe {
                self.device.destroy_buffer(self.buffer.clone(), None);
                self.device.free_memory(self.memory, None);
            }
        }
    }
}

impl Clone for VlBuffer {
    fn clone(&self) -> Self {
        Self {
            buffer: self.buffer.clone(),
            memory: self.memory.clone(),
            device: self.device.clone(),
            size: self.size.clone(),
            origin: false,
        }
    }
}

impl VlBuffer {
    pub fn new(
        device: &VlLogicalDevice,
        create_info: &vk::BufferCreateInfo,
        memory_flags: vk::MemoryPropertyFlags,
        address_required: bool,
    ) -> Self {
        let buffer = unsafe { device.device_r().create_buffer(create_info, None) }
            .expect("Failed to create buffer!");
        let requirements = unsafe {
            device
                .device_r()
                .get_buffer_memory_requirements(buffer.clone())
        };
        let mut alloc_info = vk::MemoryAllocateInfo {
            allocation_size: requirements.size,
            memory_type_index: device
                .find_memory_type(requirements.memory_type_bits, memory_flags.clone())
                .expect("Failed to find required memory type!"),
            ..Default::default()
        };
        let mut alloc_flags = vk::MemoryAllocateFlagsInfo {
            flags: vk::MemoryAllocateFlags::DEVICE_ADDRESS,
            ..Default::default()
        };
        if address_required {
            alloc_info = alloc_info.push_next(&mut alloc_flags);
        }
        let memory = unsafe { device.device_r().allocate_memory(&alloc_info, None) }
            .expect("Failed to allocate buffer memory");
        unsafe {
            device
                .device_r()
                .bind_buffer_memory(buffer.clone(), memory.clone(), 0)
                .expect("Failed to bind buffer memory");
        }
        Self {
            buffer,
            memory,
            device: device.device(),
            size: create_info.size as usize,
            origin: true,
        }
    }

    pub fn copy_from_buffer(
        &self,
        cmd: vk::CommandBuffer,
        source: &Self,
        size: DeviceSize,
        src_offset: DeviceSize,
        dst_offset: DeviceSize,
    ) {
        let copy = [vk::BufferCopy {
            size,
            src_offset,
            dst_offset,
            ..Default::default()
        }];
        unsafe {
            self.device
                .cmd_copy_buffer(cmd, source.buffer.clone(), self.buffer.clone(), &copy);
        }
    }

    pub fn copy_to_buffer(
        &self,
        cmd: vk::CommandBuffer,
        dest: &Self,
        size: DeviceSize,
        src_offset: DeviceSize,
        dst_offset: DeviceSize,
    ) {
        let copy = [vk::BufferCopy {
            size,
            src_offset,
            dst_offset,
            ..Default::default()
        }];
        unsafe {
            self.device
                .cmd_copy_buffer(cmd, self.buffer.clone(), dest.buffer.clone(), &copy);
        }
    }

    pub fn get_address(&self) -> vk::DeviceAddress {
        let address_info = vk::BufferDeviceAddressInfo {
            buffer: self.buffer.clone(),
            ..Default::default()
        };
        unsafe { self.device.get_buffer_device_address(&address_info) }
    }

    pub fn map_buffer(&self, offset: usize, map_flags: vk::MemoryMapFlags) -> *mut c_void {
        unsafe {
            self.device
                .map_memory(
                    self.memory.clone(),
                    offset as DeviceSize,
                    self.size as DeviceSize,
                    map_flags,
                )
                .expect("Failed to map buffer!")
        }
    }

    pub fn un_map(&self) {
        unsafe {
            self.device.unmap_memory(self.memory.clone());
        }
    }

    pub fn buffer(&self) -> vk::Buffer {
        self.buffer.clone()
    }
}
