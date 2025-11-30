use crate::device::image::image::VlImage;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use crate::device::swapchain::VlSwapChain;
use crate::pipelines::graphics_pipeline::config::graph_pipeline_builder::VlGraphicsPipelineBuilder;
use crate::pipelines::graphics_pipeline::VlGraphicsPipeline;
use crate::pipelines::shader::VlShader;
use crate::render_pipeline::render_image_pool::RenderImagePool;
use crate::util::vl_semaphore::VlSemaphore;
use ash::vk;
use ash::vk::RenderingAttachmentInfo;
use std::sync::{Arc, Mutex};



pub struct VlGraphicsRenderPipeline {
    swapchain: Option<Arc<Mutex<VlSwapChain>>>,
    render_images: Vec<VlImage>,
    depth_images: Vec<VlImage>,
    graphics_pipeline: VlGraphicsPipeline,

    color_clear: vk::ClearValue,
    depth_clear: vk::ClearValue,

    image_per_step_amount: u32,

    render_area: vk::Extent2D,
    scissor: vk::Rect2D,
    viewport: vk::Viewport,

    first_render: bool,
    d: bool,
}

impl VlGraphicsRenderPipeline {

    /**
    Do not populate the  color attachment info of VlGraphicsPipelineBuilder,
    it will be filled automatically!
    */
    pub fn new(
        device: &VlLogicalDevice,
        swapchain: Option<Arc<Mutex<VlSwapChain>>>,
        mut builder: VlGraphicsPipelineBuilder,
        shader: VlShader,
        render_area: vk::Extent2D,
        max_frames_in_flight: u32,
    ) -> Self {

        let mut color_images = Vec::with_capacity(if swapchain.is_none() {
            (max_frames_in_flight * builder.attachments_per_step_amount()) as usize
        } else {
            0
        });
        let mut depth_images = Vec::with_capacity(max_frames_in_flight as usize);
        if swapchain.is_some() {
            let swapchain_lock = swapchain.as_ref().unwrap().lock().unwrap();
            builder.add_color_attachment(swapchain_lock.format().format);
            unsafe {
                for _ in 0..max_frames_in_flight {
                    let depth_image = RenderImagePool::create_depth_attachment(device, (render_area.width, render_area.height));
                    builder.set_depth_attachment(depth_image.image_info().format);
                    depth_images.push(depth_image);
                }
            }
        } else {
            unsafe {
                let mut populated = false;
                for _ in 0..max_frames_in_flight {
                    for _ in 0..builder.attachments_per_step_amount() {
                        let color_image =
                        RenderImagePool::create_color_attachment(device, (render_area.width, render_area.height));
                        if !populated {
                            builder.add_color_attachment(color_image.image_info().format);
                        }
                        color_images.push(color_image);
                    }
                    let mut depth_image = RenderImagePool::create_depth_attachment(device, (render_area.width, render_area.height));
                    builder.set_depth_attachment(depth_image.image_info().format);
                    populated = true;
                    depth_image.transition_image_layout_q(
                        device
                            .find_queue_by_type_r(vk::QueueFlags::GRAPHICS)
                            .unwrap(),
                        vk::ImageLayout::UNDEFINED,
                        vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        vk::ImageAspectFlags::DEPTH,
                    );
                    depth_images.push(depth_image);
                }
            }
        }

        let graphics_pipeline = VlGraphicsPipeline::new(device.device(), shader, builder);
        let mut depth_clear = vk::ClearValue::default();
        let mut color_clear = vk::ClearValue::default();
        let mut viewport = vk::Viewport::default();
        let mut scissor = vk::Rect2D::default();
        unsafe {
            depth_clear.depth_stencil.depth = 1.0f32;
            depth_clear.depth_stencil.stencil = 0;
            color_clear.color.float32[3] = 1.0f32;
            color_clear.color.float32[2] = 1.0f32;
            viewport.width = render_area.width as f32;
            viewport.height = render_area.height as f32;
            scissor.extent.width = render_area.width;
            scissor.extent.height = render_area.height;
        }
        return Self {
            swapchain,
            render_images: Vec::new(),
            depth_images,
            graphics_pipeline,
            color_clear,
            depth_clear,
            image_per_step_amount: 1,
            render_area,
            scissor,
            viewport,
            first_render: true,
            d: false,
        };
    }

    pub fn begin(&mut self, device: &ash::Device, cmd: vk::CommandBuffer, current_image: u32) {
        let barriers = self.make_barriers(current_image);
        Self::bind_barriers(device, cmd, barriers);
        let infos = self.make_color_depth_info(current_image);
        let rendering_info = self.create_images_and_rendering_infos(infos.0.as_slice(), &infos.1);
        unsafe {
            device.cmd_begin_rendering(cmd, &rendering_info);
            device.cmd_set_viewport(cmd, 0, &[self.viewport.clone()]);
            device.cmd_set_scissor(cmd, 0, &[self.scissor.clone()]);
            device.cmd_bind_pipeline(
                cmd,
                vk::PipelineBindPoint::GRAPHICS,
                self.graphics_pipeline.get_graphics_pipeline(),
            );
        }
    }

    pub fn end_render(&mut self, device: &ash::Device, cmd: vk::CommandBuffer, current_image: u32) {
        unsafe {
            device.cmd_end_rendering(cmd);
            let barriers = self.prepare_barriers_after_rendering(current_image);
            Self::bind_barriers(device, cmd, barriers);
        }
    }

    pub fn resize(&mut self, device: &VlLogicalDevice, (width, height): (u32, u32)) {
        self.d = false;
        self.first_render = true;
        self.graphics_pipeline.resize(width, height);
        self.render_area.width = width;
        self.render_area.height = height;
        self.scissor.extent.width = self.render_area.width;
        self.scissor.extent.height = self.render_area.height;
        self.viewport.width = width as f32;
        self.viewport.height = height as f32;
        if self.swapchain.is_none() {
            self.render_images.iter_mut().for_each(|image| {
                image.resize(device, width, height);
            });
        }
        self.depth_images.iter_mut().for_each(|image| {image.resize(device, width, height);});
    }

    fn bind_barriers(
        device: &ash::Device,
        cmd: vk::CommandBuffer,
        mut barriers: Vec<vk::ImageMemoryBarrier>,
    ) {
        for i in 0..barriers.len() {
            let barrier = [barriers.remove(i)];
            let memory_barrier = Vec::new();
            let buffer_barrier = Vec::new();
            unsafe {
                device.cmd_pipeline_barrier(
                    cmd,
                    vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
                    vk::PipelineStageFlags::TOP_OF_PIPE,
                    vk::DependencyFlags::empty(),
                    memory_barrier.as_slice(),
                    buffer_barrier.as_slice(),
                    &barrier,
                );
            }
        }
    }

    fn make_barriers(&mut self, current_image: u32) -> Vec<vk::ImageMemoryBarrier<'_>> {
        if self.first_render {
            self.first_render = !(self.d && current_image == 0);
        }

        let start = current_image * self.image_per_step_amount;
        let end = (current_image + 1) * self.image_per_step_amount;

        let mut result = Vec::with_capacity(self.image_per_step_amount as usize);

        for i in start..end {
            let bar = vk::ImageMemoryBarrier::default()
                .src_access_mask(vk::AccessFlags::COLOR_ATTACHMENT_WRITE)
                .old_layout(if self.first_render {
                    if self.swapchain.is_some() {
                        vk::ImageLayout::UNDEFINED
                    } else {
                        vk::ImageLayout::GENERAL
                    }
                } else {
                    if self.swapchain.is_some() {
                        vk::ImageLayout::PRESENT_SRC_KHR
                    } else {
                        vk::ImageLayout::GENERAL
                    }
                })
                .new_layout(vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
                .image(if self.swapchain.is_some() {
                    let sw_lock = self.swapchain.as_ref().unwrap().lock().unwrap();
                    sw_lock.images()[i as usize].image()
                } else {
                    self.render_images[i as usize].image()
                })
                .subresource_range(vk::ImageSubresourceRange {
                    aspect_mask: vk::ImageAspectFlags::COLOR,
                    base_mip_level: 0,
                    level_count: 1,
                    base_array_layer: 0,
                    layer_count: 1,
                });

            result.push(bar);
        }
        self.d = true;
        result
    }

    fn prepare_barriers_after_rendering(
        &mut self,
        current_image: u32,
    ) -> Vec<vk::ImageMemoryBarrier<'_>> {
        let start = current_image * self.image_per_step_amount;
        let end = (current_image + 1) * self.image_per_step_amount;

        let mut result = Vec::with_capacity(self.image_per_step_amount as usize);

        for i in start..end {
            let bar = vk::ImageMemoryBarrier::default()
                .src_access_mask(vk::AccessFlags::COLOR_ATTACHMENT_WRITE)
                .old_layout(vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
                .new_layout(if self.swapchain.is_some() {
                    vk::ImageLayout::PRESENT_SRC_KHR
                } else {
                    vk::ImageLayout::GENERAL
                })
                .image(if self.swapchain.is_some() {
                    let sw_lock = self.swapchain.as_ref().unwrap().lock().unwrap();
                    sw_lock.images()[i as usize].image()
                } else {
                    self.render_images[i as usize].image()
                })
                .subresource_range(vk::ImageSubresourceRange {
                    aspect_mask: vk::ImageAspectFlags::COLOR,
                    base_mip_level: 0,
                    level_count: 1,
                    base_array_layer: 0,
                    layer_count: 1,
                });

            result.push(bar);
        }

        result
    }

    fn make_color_depth_info(
        &self,
        current_image: u32,
    ) -> (
        Vec<RenderingAttachmentInfo<'static>>,
        RenderingAttachmentInfo<'static>,
    ) {
        let mut color_infos = Vec::with_capacity(self.image_per_step_amount as usize);

        for i in 0..self.image_per_step_amount {
            let mut info = vk::RenderingAttachmentInfo::default()
                .image_layout(vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
                .load_op(vk::AttachmentLoadOp::CLEAR)
                .store_op(vk::AttachmentStoreOp::STORE)
                .clear_value(self.color_clear);
            if self.swapchain.is_some() {
                info.image_view = self
                    .swapchain
                    .as_ref()
                    .unwrap()
                    .lock()
                    .unwrap()
                    .image_views()[current_image as usize]
                    .base();
            } else {
                let index = current_image * self.image_per_step_amount + i;
                info.image_view = self.render_images[index as usize]
                    .get_image_views()
                    .first()
                    .unwrap()
                    .base();
            }

            color_infos.push(info);
        }

        // ------------------------------------------
        // Create depth attachment info
        // ------------------------------------------
        let depth_info = vk::RenderingAttachmentInfo::default()
            .image_layout(vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .load_op(vk::AttachmentLoadOp::CLEAR)
            .store_op(vk::AttachmentStoreOp::STORE)
            .clear_value(self.depth_clear)
            .image_view(
                self.depth_images[current_image as usize]
                    .get_image_views()
                    .first()
                    .unwrap()
                    .base(),
            );
        (color_infos, depth_info)
    }

    fn create_images_and_rendering_infos<'a>(
        &self,
        color_infos: &'a [RenderingAttachmentInfo],
        depth_info: &'a RenderingAttachmentInfo,
    ) -> vk::RenderingInfo<'a> {
        vk::RenderingInfo::default()
            .render_area(vk::Rect2D {
                offset: vk::Offset2D { x: 0, y: 0 },
                extent: self.render_area,
            })
            .layer_count(1)
            .color_attachments(color_infos)
            .depth_attachment(depth_info)
    }

}
