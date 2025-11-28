#![allow(dead_code)]
mod device;
mod instance;
mod pipelines;
mod shader;
pub mod util;
mod window;

#[cfg(test)]
mod tests {
    use crate::device::device_builder::VlDeviceBuilder;
    use crate::device::logical_device::logical_device::VlLogicalDevice;
    use crate::device::physical_device::device_suitability::VlDeviceSuitability;
    use crate::device::physical_device::physical_device::VlPhysicalDevice;
    use crate::device::swapchain::VlSwapChain;
    use crate::instance::instance::VlInstance;
    use crate::instance::instance_builder::VlInstanceBuilder;
    use crate::pipelines::graphics_pipeline::config::graph_pipeline_builder::VlGraphicsPipelineBuilder;
    use crate::pipelines::graphics_pipeline::VlGraphicsPipeline;
    use crate::pipelines::pipeline_config::pipeline_builder::{
        VlSamplerInfo,  VlVertexInput,
    };
    use crate::shader::VlShaderFileType::SrcFile;
    use crate::shader::{VlShaderCreateInfo, VlShaderLoader};
    use crate::window::Window;
    use ash::vk;
    use std::borrow::Cow;
    use std::ffi::CString;
    use std::io::Write;
    use std::path::PathBuf;
    use std::sync::{Arc, Mutex};

    #[test]
    fn it_works() {
        let shader_loader = VlShaderLoader::new();
        VlShaderLoader::add_include_directory("glsl".as_ref());
       // shader_loader.set_optimization_level(OptimizationLevel::Performance);

        let mut window = Window::new(800, 600).unwrap();

        let mut builder = VlInstanceBuilder::new();
        builder.set_app_name(CString::new("hello app name").unwrap());
        builder.set_engine_name(CString::new("Vulkan Engine").unwrap());
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
            let device =
                VlLogicalDevice::new(&instance, devices.pop().unwrap(), &dev_builder, suit);
            println!("{:?}", device.find_depth_format());
            let swap_chain = Arc::new(Mutex::new(VlSwapChain::new(
                device.clone(),
                instance.clone(),
                surface.clone(),
                800,
                600,
                true,
            )));
            let swap_chain_clone = swap_chain.clone();
            window.set_resize_callback(move |width, height| {
                swap_chain_clone
                    .lock()
                    .unwrap()
                    .recreate_swap_chain(width, height);
            });
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
                .create_shader(device.device_r(), &create_infos)
                .unwrap();

            let mut graph_builder = VlGraphicsPipelineBuilder::new(
                1,
                window.get_physical_size().0,
                window.get_physical_size().1,
                true,
            );
            graph_builder.add_color_attachment(vk::Format::R32G32B32A32_SFLOAT);
            graph_builder.set_depth_attachment(vk::Format::D32_SFLOAT);
            graph_builder.add_vertex_input(VlVertexInput {
                location: 0,
                coordinates_amount: 3,
                type_size: size_of::<f32>(),
                format: vk::Format::R32G32B32_SFLOAT,
            });
            graph_builder.add_vertex_input(VlVertexInput {
                location: 1,
                coordinates_amount: 2,
                type_size: size_of::<f32>(),
                format: vk::Format::R32G32_SFLOAT,
            });
            graph_builder.add_sample_info(VlSamplerInfo {
                binding: 0,
                descriptor_count: 1,
                shader_stages: vk::ShaderStageFlags::FRAGMENT,
            });
            graph_builder.add_sample_info(VlSamplerInfo {
                binding: 1,
                descriptor_count: 1,
                shader_stages: vk::ShaderStageFlags::FRAGMENT,
            });
            graph_builder.add_sample_info(VlSamplerInfo {
                binding: 2,
                descriptor_count: 1,
                shader_stages: vk::ShaderStageFlags::FRAGMENT,
            });

            let graphics_pipeline = VlGraphicsPipeline::new(device.device(), shader, graph_builder);

            while !window.need_close() {
                let _ = window.poll_events();
            }
            drop(window);
        }
    }
}
