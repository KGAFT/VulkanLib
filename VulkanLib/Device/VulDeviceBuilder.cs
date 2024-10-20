using Silk.NET.Vulkan;

namespace VulkanLib.Device;

public class VulDeviceBuilder
{
    private List<String> requestExtensions = new();
    private bool requirePresentSupport = false;
    private bool requireGraphicsSupport = false;
    private bool requireComputeSupport = false;
    private SurfaceKHR surfaceForPresentationCheck;
    private bool rayTracingSupport = false;

    public void addExtension(String extensionName)
    {
        requestExtensions.Add(extensionName);
    }

    public List<string> getRequestExtensions() => requestExtensions;

    public bool getRequirePresentSupport() => requirePresentSupport;

    public bool getRequireGraphicsSupport() => requireGraphicsSupport;

    public bool getRequireComputeSupport() => requireComputeSupport;

    public SurfaceKHR getSurfaceForPresentationCheck() => surfaceForPresentationCheck;

    public bool getRayTracingSupport() => rayTracingSupport;

    public void requestGraphicsSupport()
    {
        requestExtensions.Add("VK_KHR_dynamic_rendering");
        requestExtensions.Add("VK_KHR_depth_stencil_resolve");
        requestExtensions.Add("VK_KHR_create_renderpass2");
        requestExtensions.Add("VK_KHR_multiview");
        requestExtensions.Add("VK_KHR_maintenance2");
        requireGraphicsSupport = true;
    }

    public void requestRayTracingSupport()
    {
        requestExtensions.Add("VK_KHR_acceleration_structure");
        requestExtensions.Add("VK_KHR_ray_tracing_pipeline");
        requestExtensions.Add("VK_KHR_pipeline_library");
        requestExtensions.Add("VK_KHR_deferred_host_operations");
        requestExtensions.Add("VK_KHR_spirv_1_4");
        requestExtensions.Add("VK_EXT_descriptor_indexing");
        requestExtensions.Add("VK_KHR_buffer_device_address");
        requestExtensions.Add("VK_KHR_shader_float_controls");
        requestExtensions.Add("VK_KHR_maintenance3");
        requestExtensions.Add("VK_KHR_device_group");
        rayTracingSupport = true;
    }

    public bool requirePresentSupport1()
    {
        return requirePresentSupport;
    }

    public bool requireGraphicsSupport1()
    {
        return requireGraphicsSupport;
    }

    public bool requireComputeSupport1()
    {
        return requireComputeSupport;
    }

    public bool rayTracingSupport1()
    {
        return rayTracingSupport;
    }

   public void requestComputeSupport()
    {
        requireComputeSupport = true;
    }

    public void requestPresentSupport(SurfaceKHR surface)
    {
        surfaceForPresentationCheck = surface;
        requirePresentSupport = true;
        requestExtensions.Add("VK_KHR_swapchain");
    }
}