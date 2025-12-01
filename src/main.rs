#![allow(dead_code)]

mod device;
mod instance;
mod pipelines;
mod render_pipeline;
mod shader;
pub mod util;
mod window;
use crate::device::buffer::vertex_buffer::VlVertexBuffer;
use crate::device::device_builder::VlDeviceBuilder;
use crate::device::logical_device::logical_device::VlLogicalDevice;
use crate::device::physical_device::device_suitability::VlDeviceSuitability;
use crate::device::physical_device::physical_device::VlPhysicalDevice;
use crate::device::swapchain::VlSwapChain;
use crate::device::synchronization::sync_manager::VlSyncManager;
use crate::instance::instance::VlInstance;
use crate::instance::instance_builder::VlInstanceBuilder;
use crate::pipelines::graphics_pipeline::config::graph_pipeline_builder::VlGraphicsPipelineBuilder;
use crate::pipelines::pipeline_config::pipeline_builder::{
    VlPushConstantInfo, VlVertexInput,
};
use crate::render_pipeline::graphics_render_pipeline::VlGraphicsRenderPipeline;
use crate::shader::VlShaderFileType::SrcFile;
use crate::shader::{VlShaderCreateInfo, VlShaderLoader};
use crate::window::Window;
use ash::vk;
use std::borrow::Cow;
use std::ffi::{c_void, CString};
use std::io::Write;
use std::ops::Deref;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};

pub fn main() {
    let shader_loader = VlShaderLoader::new();
    VlShaderLoader::add_include_directory("glsl/".as_ref());
    // shader_loader.set_optimization_level(OptimizationLevel::Performance);

    let mut window = Window::new(800, 600).unwrap();

    let mut builder = VlInstanceBuilder::new();
    builder.set_app_name(CString::new("hello app name").unwrap());
    builder.set_engine_name(CString::new("Vulkan Engine").unwrap());
    builder.preset_hdr();
    let mut exts = Window::get_required_instance_extensions().unwrap();
    while let Some(ext) = exts.pop() {
        builder.add_enabled_extension(ext);
    }
    //builder.add_enabled_layer(CString::new("VK_LAYER_LUNARG_api_dump").unwrap());
    builder.enable_present_supported();
    builder.add_initial_debug_callback(Arc::new(Mutex::new(
        move |id: i32,
              severity: vk::DebugUtilsMessageSeverityFlagsEXT,
              m_type: vk::DebugUtilsMessageTypeFlagsEXT,
              message: Cow<str>,
              message_id: Cow<str>| {
            let msg = format!("{severity:?}: {m_type:?} [{message_id} {id}] : {message}",);
            println!("{}", msg);
            std::io::stdout().flush().unwrap();
        },
    )));
    builder.preset_for_debug();
    let instance = VlInstance::new(builder);

    let mut devices = VlPhysicalDevice::enumerate(instance.get_instance_r());
    devices.iter().for_each(|device| {
        println!(
            "{}",
            device
                .properties()
                .device_name_as_c_str()
                .unwrap()
                .to_str()
                .unwrap()
        );
    });
    let mut dev_builder = VlDeviceBuilder::new();
    let surface = unsafe {
        window.create_surface(
            instance.surface_loader().unwrap().clone(),
            instance.get_instance_r(),
        )
    }
    .expect("Failed to create Vulkan surface");
    dev_builder.require_graphics();
    dev_builder.require_compute();
    dev_builder.require_raytracing();
    dev_builder.enable_presentation(surface.clone());

    let suit = VlDeviceSuitability::is_device_suitable(&instance, &dev_builder, &devices[0]);
    if suit.0 {
        let device = Arc::new(Mutex::new(VlLogicalDevice::new(
            &instance,
            devices.pop().unwrap(),
            &dev_builder,
            suit,
        )));
        let swap_chain = Arc::new(Mutex::new(VlSwapChain::new(
            device.lock().unwrap().clone(),
            instance.clone(),
            surface.clone(),
            800,
            600,
            true,
        )));

        let create_infos = vec![
            VlShaderCreateInfo {
                path: PathBuf::from("glsl/OutputPipeline/main.vert"),
                file_type: SrcFile,
                stage: vk::ShaderStageFlags::VERTEX,
                entry_point: "main".to_string(),
            },
            VlShaderCreateInfo {
                path: PathBuf::from("glsl/OutputPipeline/main.frag"),
                file_type: SrcFile,
                stage: vk::ShaderStageFlags::FRAGMENT,
                entry_point: "main".to_string(),
            },
        ];
        let shader = shader_loader
            .create_shader(device.lock().unwrap().device_r(), &create_infos)
            .unwrap();

        let mut graph_builder = VlGraphicsPipelineBuilder::new(
            1,
            window.get_physical_size().0,
            window.get_physical_size().1,
            true,
        );
        //    graph_builder.add_color_attachment(vk::Format::R32G32B32A32_SFLOAT);
        //  graph_builder.set_depth_attachment(vk::Format::D32_SFLOAT);
        graph_builder.add_vertex_input(VlVertexInput {
            location: 0,
            coordinates_amount: 3,
            type_size: size_of::<f32>(),
            format: vk::Format::R32G32B32_SFLOAT,
        });

        graph_builder.add_push_constant(VlPushConstantInfo {
            shader_stages: vk::ShaderStageFlags::FRAGMENT,
            size: size_of::<i32>() * 4,
        });
        let frames_in_flight = swap_chain.lock().unwrap().images().len() as u32;
        let render_pipeline = Arc::new(Mutex::new(VlGraphicsRenderPipeline::new(
            &device.lock().unwrap(),
            Some(swap_chain.clone()),
            graph_builder,
            shader,
            vk::Extent2D {
                width: 800,
                height: 600,
            },
            frames_in_flight,
        )));
        let dev_lock = device.lock().unwrap();
        let sync_manager = Arc::new(Mutex::new(VlSyncManager::new(
            dev_lock.device(),
            swap_chain.clone(),
            dev_lock.find_present_queue_r().unwrap(),
            frames_in_flight,
        )));
        let sn_clocne = sync_manager.clone();
        window.set_resize_callback(move |width, height| {
            sn_clocne.lock().unwrap().resized(width, height);
        });

        let rp_clone = render_pipeline.clone();
        drop(dev_lock);
        let d_clone = device.clone();
        sync_manager
            .lock()
            .unwrap()
            .add_resize_callback(Box::new(move |width, height| {
                let d_lock = d_clone.lock().unwrap();
                rp_clone
                    .lock()
                    .unwrap()
                    .resize(d_lock.deref(), (width, height))
            }));

        let mut cur_cmd: u32 = 0;

        let triangle_vertices: [f32; 9] = [
            //    X      Y     Z
            0.0, 0.5, 0.0, // top
            -0.5, -0.5, 0.0, // bottom-left
            0.5, -0.5, 0.0, // bottom-right
        ];

        let vertex_buffer = VlVertexBuffer::new(
            device.lock().unwrap().deref(),
            (triangle_vertices.as_slice() as &_ as *const _) as *const c_void,
            3,
            size_of::<f32>() * 3,
            vk::Format::R32G32B32_SFLOAT,
            false
        );

        while !window.need_close() {
            let cmd = sync_manager
                .lock()
                .unwrap()
                .begin_render(&mut cur_cmd)
                .unwrap();
            render_pipeline.lock().unwrap().begin(
                device.lock().unwrap().device_r(),
                cmd.clone(),
                cur_cmd,
            );

            vertex_buffer.bind(cmd.clone());
            vertex_buffer.draw_all(cmd.clone());

            render_pipeline.lock().unwrap().end_render(
                device.lock().unwrap().device_r(),
                cmd,
                cur_cmd,
            );
            sync_manager.lock().unwrap().end_render();
            let _ = window.poll_events();
        }
        window.clear_resize_callbacks();

        drop(render_pipeline);
        drop(sync_manager);
        drop(vertex_buffer);
        drop(swap_chain);
        unsafe { let _ = device.lock().unwrap().device_r().device_wait_idle(); }
        drop(window);

    }
}
