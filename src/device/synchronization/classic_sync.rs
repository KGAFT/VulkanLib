use ash::vk;
use ash::khr::swapchain;

pub struct VlMultiFrameSync {
    max_frames_in_flight: usize,

    image_available_semaphores: Vec<vk::Semaphore>,
    render_finished_semaphores: Vec<vk::Semaphore>,
    in_flight_fences: Vec<vk::Fence>,
    images_in_flight: Vec<vk::Fence>, // Per-image fence

    device: ash::Device,
    present_queue: vk::Queue,

    current_frame: usize,
}

impl VlMultiFrameSync {
    pub fn new(device: ash::Device, present_queue: vk::Queue, max_frames_in_flight: u32) -> Self {
        let max_frames_in_flight = max_frames_in_flight as usize;

        let mut s = Self {
            max_frames_in_flight,
            image_available_semaphores: Vec::new(),
            render_finished_semaphores: Vec::new(),
            in_flight_fences: Vec::new(),
            images_in_flight: Vec::new(),
            device,
            present_queue,
            current_frame: 0,
        };

        s.create_sync_objects();
        s
    }

    fn create_sync_objects(&mut self) {
        let sem_info = vk::SemaphoreCreateInfo::default();
        let fence_info = vk::FenceCreateInfo::default()
            .flags(vk::FenceCreateFlags::SIGNALED);

        unsafe {
            for _ in 0..self.max_frames_in_flight {
                let image_avail = self.device.create_semaphore(&sem_info, None).unwrap();
                let render_finish = self.device.create_semaphore(&sem_info, None).unwrap();
                let in_flight = self.device.create_fence(&fence_info, None).unwrap();

                self.image_available_semaphores.push(image_avail);
                self.render_finished_semaphores.push(render_finish);
                self.in_flight_fences.push(in_flight);
                self.images_in_flight.push(vk::Fence::null());
            }
        }
    }

    pub fn prepare_for_next_image(
        &mut self,
        swapchain: vk::SwapchainKHR,
        swapchain_loader: &swapchain::Device,
    ) -> u32 {
        unsafe {
            self.device
                .wait_for_fences(
                    &[self.in_flight_fences[self.current_frame]],
                    true,
                    u64::MAX,
                )
                .expect("Failed waiting for frame fence");

            let (image_index, _suboptimal) = swapchain_loader
                .acquire_next_image(
                    swapchain,
                    u64::MAX,
                    self.image_available_semaphores[self.current_frame],
                    vk::Fence::null(),
                )
                .expect("Failed acquiring next image");

            image_index
        }
    }

    pub fn submit_command_buffers(
        &mut self,
        command_buffer: vk::CommandBuffer,
        swapchain: vk::SwapchainKHR,
        swapchain_loader: &swapchain::Device,
        current_image: u32,
    ) {
        unsafe {
            let img_fence = self.images_in_flight[current_image as usize];
            if img_fence != vk::Fence::null() {
                self.device
                    .wait_for_fences(&[img_fence], true, u64::MAX)
                    .expect("Failed to wait for image fence");
            }

            self.images_in_flight[current_image as usize] =
                self.in_flight_fences[self.current_frame];

            self.device
                .reset_fences(&[self.in_flight_fences[self.current_frame]])
                .expect("Failed to reset frame fence");

            let wait_stages = [vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT];

            let wait_semaphores = [self.image_available_semaphores[self.current_frame]];
            let command_buffers = [command_buffer];
            let signal_semaphores = [self.render_finished_semaphores[self.current_frame]];
            let submit_info = vk::SubmitInfo::default()
                .wait_semaphores(&wait_semaphores)
                .wait_dst_stage_mask(&wait_stages)
                .command_buffers(&command_buffers)
                .signal_semaphores(&signal_semaphores);

            self.device
                .queue_submit(
                    self.present_queue,
                    &[submit_info],
                    self.in_flight_fences[self.current_frame],
                )
                .expect("Failed to submit queue");

            let wait_semaphores2 = [self.render_finished_semaphores[self.current_frame]];
            let swapchains = [swapchain];
            let indices = [current_image];
            let present_info = vk::PresentInfoKHR::default()
                .wait_semaphores(&wait_semaphores2)
                .swapchains(&swapchains)
                .image_indices(&indices);

            swapchain_loader
                .queue_present(self.present_queue, &present_info)
                .expect("Failed to present");

            self.current_frame = (self.current_frame + 1) % self.max_frames_in_flight;
        }
    }

    pub fn destroy(&mut self){
        unsafe {
            self.device.device_wait_idle().ok();

            for i in 0..self.max_frames_in_flight {
                self.device
                    .destroy_semaphore(self.image_available_semaphores[i], None);
                self.device
                    .destroy_semaphore(self.render_finished_semaphores[i], None);
                self.device.destroy_fence(self.in_flight_fences[i], None);
            }
        }
    }
}

impl Drop for VlMultiFrameSync {
    fn drop(&mut self) {
        self.destroy();
    }
}
