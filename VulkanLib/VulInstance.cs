using System.Runtime.InteropServices;
using Silk.NET.Core;
using Silk.NET.Core.Native;
using VulkanLib.InstanceLogger;
using VulkanLib.ObjectManagement;

namespace VulkanLib;

using Silk.NET.Vulkan;
using Silk.NET.Vulkan.Extensions.EXT;

public class VulInstance : DestroyableObject
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
        DebugUtilsMessengerCreateInfoEXT createInfoExt = new();
        if (builder.getDebugLogging())
        {
            logger = new();
            foreach (var vulInstanceLoggerCallback in builder.getLoggerCallbacks())
            {
                logger.registerCallback(vulInstanceLoggerCallback);
            }
            logger.describeLogger(ref createInfoExt);
            createInfo.PNext = &createInfoExt;
        }
        if (vk.CreateInstance(createInfo, null, out instance) != Result.Success)
        {
            throw new Exception("failed to create instance!");
        }
        enabledLayers = builder.getLayers();
        if (builder.getDebugLogging())
        {
            ExtDebugUtils debugUtils;
            if (!vk!.TryGetInstanceExtension(instance, out debugUtils)) throw new Exception("Failed to create debug utils!");
          
            logger.initialize(instance, debugUtils);
        }
        
    }
    private Instance instance;
    private Vk? vk;
    private List<String> enabledLayers;
    private VulInstanceLogger logger = null;

    public Instance getBase() => instance;
    
    public VulInstanceLogger getLogger() => logger;
    
    public override unsafe void destroy()
    {
        base.destroy();
        logger.destroy();
        vk.DestroyInstance(instance, null);
    }
}