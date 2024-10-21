// See https://aka.ms/new-console-template for more information


using System.Runtime.InteropServices;
using VulkanLib;
using Silk.NET.Windowing;
using Silk.NET.Maths;
using Silk.NET.Core.Native;
using Silk.NET.Vulkan;
using VulkanLib.Device;
using VulkanLib.Device.LogicalDevice;
using VulkanLib.Device.PhysicalDevice;
using VulkanLib.InstanceLogger;
using VulkanLib.ObjectManagement;

unsafe
{
    IWindow window = initWindow();
    VulInstanceBuilder builder = new VulInstanceBuilder();

    foreach(string element in getRequiredWindowExtensions(window))
    {
        builder.addExtension(element);
    }
    builder.presetForDebugging();
    builder.addLoggerCallback(new DefaultVulkanLoggerCallback());
    VulInstance instance = new(builder);
    var devices = VulPhysicalDevice.getDevices(instance);
    var devBuilder = new VulDeviceBuilder();
    var surface = window!.VkSurface!.Create<AllocationCallbacks>(instance.getBase().ToHandle(), null).ToSurface();

    devBuilder.requestGraphicsSupport();
    devBuilder.requestComputeSupport();
    devBuilder.requestRayTracingSupport();
    devBuilder.requestPresentSupport(surface);
    List<VulPhysicalDevice> suitableDevices = new();
    List<DeviceSuitabilityResults> suitRes = new List<DeviceSuitabilityResults>();
    foreach (var vulPhysicalDevice in devices)
    {
        unsafe
        {
            PhysicalDeviceProperties props = vulPhysicalDevice.getProperties();
            var results = new DeviceSuitabilityResults();
            if (DeviceSuitability.isDeviceSuitable(instance, devBuilder, vulPhysicalDevice, ref results))
            {
                Console.WriteLine(Marshal.PtrToStringAnsi((nint)props.DeviceName));
                suitableDevices.Add(vulPhysicalDevice);
                suitRes.Add(results);
            }
        }
    }
    
    if(suitableDevices.Count == 0)
        throw new Exception("No suitable devices found");
    VulLogicalDevice device = new(instance, suitableDevices[0], devBuilder, suitRes[0]);

    window.Run();

    unsafe string[] getRequiredWindowExtensions(IWindow window)
    {
        var glfwExtensions = window!.VkSurface!.GetRequiredExtensions(out var glfwExtensionCount);
        var extensions = SilkMarshal.PtrToStringArray((nint)glfwExtensions, (int)glfwExtensionCount);
        return extensions;
    }

    unsafe IWindow initWindow()
    {
        var options = WindowOptions.DefaultVulkan with
        {
            Size = new Vector2D<int>(1920, 1080),
            Title = "Vulkan",
        };

        IWindow window = Window.Create(options);
        window.Initialize();

        if (window.VkSurface is null)
        {
            throw new Exception("Windowing platform doesn't support Vulkan.");
        }
        return window;
    }
}