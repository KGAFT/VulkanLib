use crate::device::device_builder::VlDeviceBuilder;
use crate::device::logical_device::logical_queue::VlLogicalQueue;
use crate::device::physical_device::device_suitability::VlQueueFamilyInfo;
use crate::device::physical_device::physical_device::VlPhysicalDevice;
use crate::instance::instance::VlInstance;
use crate::util::c_string_vec_to_ptr;
use ash::{vk, Instance};
use std::ffi::c_void;
use std::sync::{Arc, Mutex};

static QUEUE_PRIORITY: f32 = 1.0f32;

pub struct VlLogicalDevice {
    device: ash::Device,
    queues: Vec<VlLogicalQueue>,
    base_device: VlPhysicalDevice,
    mem_properties: Arc<Mutex<Option<vk::PhysicalDeviceMemoryProperties>>>,
    instance: VlInstance,
    original: bool,
}

impl Clone for VlLogicalDevice {
    fn clone(&self) -> Self {
        Self {
            device: self.device.clone(),
            queues: self.queues.clone(),
            base_device: self.base_device.clone(),
            mem_properties: self.mem_properties.clone(),
            instance: self.instance.clone(),
            original: false,
        }
    }
}

impl VlLogicalDevice {
    pub fn new(
        instance: &VlInstance,
        base_device: VlPhysicalDevice,
        builder: &VlDeviceBuilder,
        suit_res: (bool, Vec<VlQueueFamilyInfo>),
    ) -> Self {
        let queue_infos = Self::suit_res_to_c_info(&suit_res.1);
        let mut features = vk::PhysicalDeviceFeatures::default();
        features.shader_int64 = vk::TRUE;

        let mut dynamic_rendering_feature =
            vk::PhysicalDeviceDynamicRenderingFeaturesKHR::default();
        let mut accel_structure = vk::PhysicalDeviceAccelerationStructureFeaturesKHR::default();

        dynamic_rendering_feature.dynamic_rendering = vk::TRUE;

        let mut new_features = vk::PhysicalDeviceVulkan12Features::default();
        new_features.buffer_device_address = vk::TRUE;
        new_features.descriptor_indexing = vk::TRUE;
        new_features.runtime_descriptor_array = vk::TRUE;
        new_features.shader_sampled_image_array_non_uniform_indexing = vk::TRUE;
        new_features.uniform_and_storage_buffer8_bit_access = vk::TRUE;
        new_features.storage_buffer8_bit_access = vk::TRUE;
        new_features.shader_int8 = vk::TRUE;

        features.fragment_stores_and_atomics = vk::TRUE;
        features.sampler_anisotropy = vk::TRUE;

        dynamic_rendering_feature.p_next = &mut new_features as *mut _ as *mut c_void;

        let mut ray_tracing_pipeline_features_khr =
            vk::PhysicalDeviceRayTracingPipelineFeaturesKHR::default();

        ray_tracing_pipeline_features_khr.s_type =
            vk::StructureType::PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

        ray_tracing_pipeline_features_khr.ray_tracing_pipeline = vk::TRUE;
        if builder.require_raytracing() {
            accel_structure.acceleration_structure = vk::TRUE;
            new_features.p_next = &mut accel_structure as *mut _ as *mut c_void;
            accel_structure.p_next =
                &mut ray_tracing_pipeline_features_khr as *mut _ as *mut c_void;
        }
        let layers = c_string_vec_to_ptr(instance.enabled_layers());
        let extensions = builder.get_extensions();
        let mut dev_create_info = vk::DeviceCreateInfo {
            ..Default::default()
        }
        .queue_create_infos(queue_infos.as_slice())
        .enabled_layer_names(layers.as_slice())
        .enabled_extension_names(extensions.as_slice())
        .enabled_features(&features)
        .push_next(&mut dynamic_rendering_feature);
        let device = unsafe {
            instance.get_instance_r().create_device(
                base_device.inner().clone(),
                &dev_create_info,
                None,
            )
        }
        .expect("Failed to create device");
        let mut queues = Vec::new();
        unsafe {
            suit_res.1.iter().for_each(|queue_info| {
                let queue = device.get_device_queue(queue_info.index, 0);
                queues.push(VlLogicalQueue::new(
                    queue,
                    queue_info.index,
                    queue_info.support_present,
                    queue_info.properties.queue_flags,
                    &device,
                ));
            });
        }
        Self {
            device,
            base_device,
            queues,
            mem_properties: Arc::new(Mutex::new(None)),
            original: true,
            instance: instance.clone(),
        }
    }

    pub fn find_queue_by_type_r(&self, queue_type: vk::QueueFlags) -> Option<&VlLogicalQueue> {
        for x in self.queues.iter() {
            if x.queue_type().contains(queue_type) {
                return Some(x);
            }
        }
        None
    }

    pub fn find_queue_by_type(&self, queue_type: vk::QueueFlags) -> Option<VlLogicalQueue> {
        for x in self.queues.iter() {
            if x.queue_type().contains(queue_type) {
                return Some(x.clone());
            }
        }
        None
    }

    pub fn find_present_queue_r(&self) -> Option<&VlLogicalQueue> {
        for x in self.queues.iter() {
            if x.support_present() {
                return Some(x);
            }
        }
        None
    }

    pub fn find_present_queue(&self) -> Option<VlLogicalQueue> {
        for x in self.queues.iter() {
            if x.support_present() {
                return Some(x.clone());
            }
        }
        None
    }

    pub fn find_depth_format(&self) -> Option<vk::Format> {
        let candidates = vec![vk::Format::D32_SFLOAT, vk::Format::D32_SFLOAT_S8_UINT, vk::Format::D24_UNORM_S8_UINT];
        self.find_supported_format(candidates.as_slice(), vk::ImageTiling::OPTIMAL, vk::FormatFeatureFlags::DEPTH_STENCIL_ATTACHMENT)
    }

    pub fn fnd_memory_type(&self, mut type_filter: u32, properties: vk::MemoryPropertyFlags) -> Option<u32>{
        let mut lock = self.mem_properties.lock().unwrap();
        if lock.is_none() {
            unsafe { *lock = Some(self.instance.get_instance_r().get_physical_device_memory_properties(self.base_device.inner())); }
        }
        let mem_properties = lock.as_ref().unwrap();
        for (i, memory_type) in mem_properties
            .memory_types[..mem_properties.memory_type_count as usize]
            .iter()
            .enumerate()
        {
            if (type_filter & (1 << i)) != 0
                && memory_type.property_flags.contains(properties)
            {
                return Some(i as u32);
            }
        }
        None
    }

    pub fn device_r(&self) -> &ash::Device {
        &self.device
    }

    pub fn base_device_r(&self) -> &VlPhysicalDevice {
        &self.base_device
    }

    pub fn device(&self) -> ash::Device {
        self.device.clone()
    }

    pub fn base_device(&self) -> VlPhysicalDevice {
        self.base_device.clone()
    }

    fn suit_res_to_c_info(suit_res: &Vec<VlQueueFamilyInfo>) -> Vec<vk::DeviceQueueCreateInfo> {
        let mut res = Vec::with_capacity(suit_res.len());
        suit_res.iter().for_each(|queue_family| {
            let create_info = vk::DeviceQueueCreateInfo {
                queue_family_index: queue_family.index,
                p_queue_priorities: &QUEUE_PRIORITY,
                queue_count: 1,
                ..Default::default()
            };
            res.push(create_info);
        });
        res
    }

    fn find_supported_format(
        &self,
        candidates: &[vk::Format],
        tiling: vk::ImageTiling,
        features: vk::FormatFeatureFlags,
    ) -> Option<vk::Format> {
        for &format in candidates {
            let props = unsafe {
                self.instance.get_instance_r().get_physical_device_format_properties(self.base_device.inner(), format)
            };

            let supported = match tiling {
                vk::ImageTiling::LINEAR => props.linear_tiling_features.contains(features),
                vk::ImageTiling::OPTIMAL => props.optimal_tiling_features.contains(features),
                _ => false,
            };

            if supported {
                return Some(format);
            }
        }

        None
    }

}

impl Drop for VlLogicalDevice {
    fn drop(&mut self) {
        if self.original {
            while let Some(queue) = self.queues.pop() {
                std::mem::drop(queue);
            }
            unsafe {
                let _ = self.device.device_wait_idle();
            }
            unsafe {
                self.device.destroy_device(None);
            }
        }
    }
}
