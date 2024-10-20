// See https://aka.ms/new-console-template for more information



using VulkanLib;
using Silk.NET.Windowing;
using Silk.NET.Maths;
using Silk.NET.Core.Native;
using Silk.NET.Vulkan;
using VulkanLib.InstanceLogger;
using VulkanLib.ObjectManagement;

IWindow window = initWindow();
VulInstanceBuilder builder = new VulInstanceBuilder();

foreach(string element in getRequiredWindowExtensions(window))
{
    builder.addExtension(element);
}
builder.presetForDebugging();
builder.addLoggerCallback(new DefaultVulkanLoggerCallback());
VulInstance instance = new(builder);
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