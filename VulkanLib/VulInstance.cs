using System.Runtime.InteropServices;
using Silk.NET.Core;
using Silk.NET.Core.Native;

namespace VulkanLib;

using Silk.NET.Vulkan;

public class VulInstance
{
    public unsafe VulInstance(VulInstanceBuilder builder)
    {
        vk = Vk.GetApi();
        ApplicationInfo appInfo = new()
        {
            SType = StructureType.ApplicationInfo,
            PApplicationName = (byte*)Marshal.StringToHGlobalAnsi(builder.getApplicationName()),
            ApplicationVersion = new Version32(1, 0, 0),
            PEngineName = (byte*)Marshal.StringToHGlobalAnsi("VulkanLib"),
            EngineVersion = new Version32(1, 0, 0),
            ApiVersion = Vk.Version13
        };
        InstanceCreateInfo createInfo = new()
        {
            SType = StructureType.InstanceCreateInfo,
            PApplicationInfo = &appInfo
        };
        createInfo.PpEnabledExtensionNames = (byte**)SilkMarshal.StringArrayToMemory(builder.getExtensions());
        createInfo.PpEnabledLayerNames = (byte**)SilkMarshal.StringArrayToMemory(builder.getLayers());
        createInfo.EnabledExtensionCount = (uint)builder.getExtensions().Count;
        createInfo.EnabledLayerCount = (uint)builder.getLayers().Count;
        if (vk.CreateInstance(createInfo, null, out instance) != Result.Success)
        {
            throw new Exception("failed to create instance!");
        }
        
    }
    private Instance instance;
    private Vk? vk;
    
}