use ash::{vk, Instance};

#[derive(Clone)]
pub struct VlPhysicalDevice {
    inner: vk::PhysicalDevice,
    properties: vk::PhysicalDeviceProperties,
    features: vk::PhysicalDeviceFeatures,
    extension_properties: Vec<vk::ExtensionProperties>,
    queue_properties: Vec<vk::QueueFamilyProperties>,
}

impl VlPhysicalDevice {
    pub fn enumerate(instance: &Instance) -> Vec<Self> {
        let mut pre_res = unsafe { instance.enumerate_physical_devices() }
            .expect("Failed to enumerate physical devices");
        let mut res: Vec<VlPhysicalDevice> = Vec::with_capacity(pre_res.len());
        while let Some(dev) = pre_res.pop() {
            res.push(Self::new(instance, dev));
        }
        res
    }

    pub fn new(instance: &ash::Instance, inner: vk::PhysicalDevice) -> Self {
        let ext_props = unsafe { instance.enumerate_device_extension_properties(inner.clone()) }
            .expect("failed to fetch device extensions");
        let features = unsafe { instance.get_physical_device_features(inner.clone()) };
        let properties = unsafe { instance.get_physical_device_properties(inner.clone()) };
        let queue_props =
            unsafe { instance.get_physical_device_queue_family_properties(inner.clone()) };
        Self {
            inner,
            properties,
            features,
            extension_properties: ext_props,
            queue_properties: queue_props,
        }
    }

    pub fn inner(&self) -> vk::PhysicalDevice {
        self.inner.clone()
    }

    pub fn properties(&self) -> &vk::PhysicalDeviceProperties {
        &self.properties
    }

    pub fn features(&self) -> &vk::PhysicalDeviceFeatures {
        &self.features
    }

    pub fn extension_properties(&self) -> &Vec<vk::ExtensionProperties> {
        &self.extension_properties
    }

    pub fn queue_properties(&self) -> &Vec<vk::QueueFamilyProperties> {
        &self.queue_properties
    }

    pub fn raytracing_pipeline_properties(
        & self,
        instance: &Instance,
    ) -> vk::PhysicalDeviceRayTracingPipelinePropertiesKHR<'_> {
        let mut res = vk::PhysicalDeviceRayTracingPipelinePropertiesKHR::default();
        let mut prop2 = vk::PhysicalDeviceProperties2::default();
        prop2 = prop2.push_next(&mut res);
        unsafe {
            instance.get_physical_device_properties2(self.inner.clone(), &mut prop2);
        }
        res
    }
}
