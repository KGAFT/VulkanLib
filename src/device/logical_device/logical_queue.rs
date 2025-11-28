use ash::vk;
use ash::vk::{Fence, Queue};

pub struct VlLogicalQueue {
    queue: vk::Queue,
    support_present: bool,
    queue_type: vk::QueueFlags,
    command_pool: vk::CommandPool,
    index: u32,
    device: ash::Device,
    original: bool,
}

impl Clone for VlLogicalQueue {
    fn clone(&self) -> Self {
        Self {
            queue: self.queue.clone(),
            support_present: self.support_present.clone(),
            queue_type: self.queue_type.clone(),
            command_pool: self.command_pool.clone(),
            index: self.index.clone(),
            device: self.device.clone(),
            original: false,
        }
    }
}

impl VlLogicalQueue {
    pub fn new(
        queue: Queue,
        index: u32,
        support_present: bool,
        queue_flags: vk::QueueFlags,
        device: &ash::Device,
    ) -> Self {
        let pool_info = vk::CommandPoolCreateInfo {
            queue_family_index: index,
            flags: vk::CommandPoolCreateFlags::TRANSIENT
                | vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER,
            ..Default::default()
        };
        let pool = unsafe {
            device
                .create_command_pool(&pool_info, None)
                .expect("failed to create command pool")
        };

        Self {
            queue,
            support_present,
            queue_type: queue_flags,
            command_pool: pool,
            index,
            device: device.clone(),
            original: true,
        }
    }

    
    pub fn begin_single_time_command(&self) -> vk::CommandBuffer {
        let alloc_info = vk::CommandBufferAllocateInfo {
            level: vk::CommandBufferLevel::PRIMARY,
            command_pool: self.command_pool.clone(),
            command_buffer_count: 1,
            ..Default::default()
        };
        let cmd = unsafe { self.device.allocate_command_buffers(&alloc_info) }
            .expect("failed to allocate command buffers")
            .pop()
            .unwrap();
        let begin_info = vk::CommandBufferBeginInfo {
            flags: vk::CommandBufferUsageFlags::ONE_TIME_SUBMIT,
            ..Default::default()
        };
        unsafe {
            self.device
                .begin_command_buffer(cmd.clone(), &begin_info)
                .expect("failed to begin cmd");
        }
        cmd
    }

    pub fn end_single_time_command(&self, cmd: vk::CommandBuffer) {
        unsafe {
            self.device
                .end_command_buffer(cmd.clone())
                .expect("failed to end cmd");
        }
        let submit_info = vec![vk::SubmitInfo {
            p_command_buffers: &cmd,
            command_buffer_count: 1,
            ..Default::default()
        }];
        unsafe {
            self.device
                .queue_submit(self.queue.clone(), submit_info.as_slice(), Fence::null())
                .expect("failed to submit queue");
        }
        unsafe {
            let _ = self.device.queue_wait_idle(self.queue.clone());
        }
        let cmd = vec![cmd];
        unsafe {
            self.device
                .free_command_buffers(self.command_pool.clone(), cmd.as_slice());
        }
    }

    pub fn queue(&self) -> vk::Queue {
        self.queue
    }

    pub fn support_present(&self) -> bool {
        self.support_present
    }

    pub fn queue_type(&self) -> vk::QueueFlags {
        self.queue_type
    }

    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn command_pool(&self) -> vk::CommandPool {
        self.command_pool
    }
}

impl Drop for VlLogicalQueue {
    fn drop(&mut self) {
        if self.original {
            unsafe {
                self.device.destroy_command_pool(self.command_pool, None);
            }
        }
    }
}
