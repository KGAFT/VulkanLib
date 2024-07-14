package com.kgaft.VulkanLib.Device;

import java.util.ArrayList;
import java.util.Collection;

public class DeviceBuilder {
    private ArrayList<String> requestExtensions = new ArrayList<>();
    private boolean requirePresentSupport = false;
    private boolean requireGraphicsSupport = false;
    private boolean requireComputeSupport = false;
    private long surfaceForPresentationCheck;
    private boolean rayTracingSupport = false;

    public void addExtension(String extensionName) {
        requestExtensions.add(extensionName);
    }
    public void addExtensions(Collection<String> extensions){
        requestExtensions.addAll(extensions);
    }
    public void requestGraphicSupport(){
        requestExtensions.add("VK_KHR_dynamic_rendering");
        requestExtensions.add("VK_KHR_depth_stencil_resolve");
        requestExtensions.add("VK_KHR_create_renderpass2");
        requestExtensions.add("VK_KHR_multiview");
        requestExtensions.add("VK_KHR_maintenance2");
        requireGraphicsSupport = true;
    }

    public void requestRayTracingSupport() {
        requestExtensions.add("VK_KHR_acceleration_structure");
        requestExtensions.add("VK_KHR_ray_tracing_pipeline");
        requestExtensions.add("VK_KHR_ray_query");
        requestExtensions.add("VK_KHR_pipeline_library");
        requestExtensions.add("VK_KHR_deferred_host_operations");
        requestExtensions.add("VK_KHR_spirv_1_4");
        requestExtensions.add("VK_EXT_descriptor_indexing");
        requestExtensions.add("VK_KHR_buffer_device_address");
        requestExtensions.add("VK_KHR_shader_float_controls");
        requestExtensions.add("VK_KHR_maintenance3");
        requestExtensions.add("VK_KHR_device_group");
        rayTracingSupport = true;
    }

    public void requestComputeSupport() {
        requireComputeSupport = true;
    }

    public void requestPresentSupport(long surface) {
        surfaceForPresentationCheck = surface;
        requirePresentSupport = true;
        requestExtensions.add("VK_KHR_swapchain");
    }

    public ArrayList<String> getRequestExtensions() {
        return requestExtensions;
    }

    public boolean isRequirePresentSupport() {
        return requirePresentSupport;
    }

    public boolean isRequireGraphicsSupport() {
        return requireGraphicsSupport;
    }

    public boolean isRequireComputeSupport() {
        return requireComputeSupport;
    }

    public long getSurfaceForPresentationCheck() {
        return surfaceForPresentationCheck;
    }

    public boolean isRayTracingSupport() {
        return rayTracingSupport;
    }
}
