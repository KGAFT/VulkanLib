use ash::vk;
use std::sync::{Arc, Mutex};
use ash::vk::CommandPool;
use crate::device::logical_device::logical_queue::VlLogicalQueue;
use crate::device::swapchain::VlSwapChain;
use crate::device::synchronization::classic_sync::VlMultiFrameSync;

pub struct VlSyncManager<F>
where
    F: FnMut(u32, u32),
{
    command_buffers: Vec<vk::CommandBuffer>,
    sync: VlMultiFrameSync,
    device: ash::Device,
    swapchain: Arc<Mutex<VlSwapChain>>,
    queue:  vk::Queue,
    current_cmd: u32,
    resize_callbacks: Vec<Box<F>>,
    stop: bool,
    width: u32,
    height: u32,
    is_resized: bool,
    command_pool: CommandPool
}

impl<F> VlSyncManager<F>
where
    F: FnMut(u32, u32),
{
    pub fn new(
        device: ash::Device,
        swapchain: Arc<Mutex<VlSwapChain>>,
        queue: &VlLogicalQueue,
        max_frames: u32,
    ) -> Self {
        let mut mgr = Self {
            command_buffers: Vec::new(),
            sync: VlMultiFrameSync::new(device.clone(), queue.queue().clone(), max_frames),
            device,
            swapchain,
            queue: queue.queue().clone(),
            current_cmd: 0,
            resize_callbacks: Vec::new(),
            stop: false,
            width: 0,
            height: 0,
            is_resized: false,
            command_pool: queue.command_pool().clone()
        };

        mgr.create_command_buffers(max_frames);
        mgr
    }

    fn create_command_buffers(&mut self, max_frames: u32) {
        let alloc = vk::CommandBufferAllocateInfo::default()
            .command_pool(self.command_pool)
            .level(vk::CommandBufferLevel::PRIMARY)
            .command_buffer_count(max_frames);

        self.command_buffers = unsafe {
            self.device
                .allocate_command_buffers(&alloc)
                .expect("Failed to allocate command buffers")
        };
    }

    pub fn begin_render(&mut self, out_cmd: &mut u32) -> Option<vk::CommandBuffer> {
        let begin_info =  vk::CommandBufferBeginInfo::default();
        if self.stop {
            return None;
        }

        if self.is_resized {
            self.stop = true;
            unsafe {
                self.device.device_wait_idle().unwrap();
            }
            self.swapchain.lock().unwrap().recreate_swap_chain(self.width, self.height);

            for cb in self.resize_callbacks.iter_mut() {
                cb(self.width, self.height);
            }

            self.is_resized = false;
            self.stop = false;
        }
        let swap_chain_lock = self.swapchain.lock().unwrap();
        self.current_cmd = self.sync.prepare_for_next_image(swap_chain_lock.swap_chain(), swap_chain_lock.swap_chain_loader());
        drop(swap_chain_lock);
        *out_cmd = self.current_cmd;

        let cmd = self.command_buffers[self.current_cmd as usize];

        unsafe {
            self.device
                .begin_command_buffer(cmd, &begin_info)
                .expect("Begin command buffer failed");
        }

        Some(cmd)
    }

    pub fn end_render(&mut self) {
        if self.stop {
            return;
        }

        let cmd = self.command_buffers[self.current_cmd as usize];

        unsafe {
            let _ = self.device.end_command_buffer(cmd);
        }
        let swap_chain_lock = self.swapchain.lock().unwrap();
        self.sync.submit_command_buffers(
            self.command_buffers[self.current_cmd as usize],
            swap_chain_lock.swap_chain(),
            swap_chain_lock.swap_chain_loader(),
            self.current_cmd,
        );
    }

    pub fn add_resize_callback(&mut self, callback: F) {
        self.resize_callbacks.push(Box::new(callback));
    }

    pub fn resized(&mut self, w: u32, h: u32) {
        self.width = w;
        self.height = h;
        self.is_resized = true;
    }

    pub fn is_stop(&self) -> bool {
        self.stop
    }

    pub fn set_stop(&mut self, stop: bool) {
        self.stop = stop;
    }

    pub fn destroy(&mut self) {
    }
}
