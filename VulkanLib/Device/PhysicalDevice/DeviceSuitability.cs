using System.Runtime.InteropServices;
using Silk.NET.Core;
using Silk.NET.Vulkan;

namespace VulkanLib.Device.PhysicalDevice;

public struct QueueFamilyInfo(uint index, QueueFamilyProperties properties, bool supportPresentation)
{
    uint index;
    QueueFamilyProperties properties;
    bool supportPresentation;
};

public struct DeviceSuitabilityResults {
    public List<QueueFamilyInfo> queuesInfo;
};

public class DeviceSuitability
{
     public static unsafe bool isDeviceSuitable(VulInstance instance, VulDeviceBuilder builder, VulPhysicalDevice device,
                                 ref DeviceSuitabilityResults pOutput) {
         
         var availableExtensionNames = device.getExtensionProperties().Select(extension => Marshal.PtrToStringAnsi((IntPtr)extension.ExtensionName)).ToHashSet();
         bool extensionCheck = builder.getRequestExtensions().All(availableExtensionNames.Contains);
         if (!extensionCheck)
         {
             return false;
         }
        bool graphicsFound = false;
        bool presentFound = false;
        bool computeFound = false;
        uint queueCounter = 0;
        QueueFamilyInfo info = new QueueFamilyInfo();
        foreach (var item in device.getQueueProperties())
        {
            if (builder.getRequireGraphicsSupport()) {
                if ((item.QueueFlags & QueueFlags.GraphicsBit) > 0) {
                    graphicsFound = true;
                    info = new( queueCounter, item, false);
                }
            }
            if (builder.getRequireComputeSupport()) {
                if ((item.QueueFlags & QueueFlags.ComputeBit) > 0) {
                    computeFound = true;
                    info = new( queueCounter, item, false);
                }
            }
            if (builder.getRequirePresentSupport())
            {
                Bool32 checkPres = new();
                instance.getSwapchainExtension().GetPhysicalDeviceSurfaceSupport(device.getBase(), queueCounter, builder.getSurfaceForPresentationCheck(), &checkPres);
                if (checkPres)
                {
                    presentFound = true;
                    info = new(queueCounter, item, true);
                }
               
            }

            pOutput.queuesInfo = new();
            pOutput.queuesInfo.Add(info);
            
            if (graphicsFound == builder.getRequireGraphicsSupport() && presentFound == builder.getRequirePresentSupport() &&
                computeFound == builder.getRequireComputeSupport()) {
                break;
            }
            queueCounter++;
        }
        
        if (!(graphicsFound == builder.getRequireGraphicsSupport() && presentFound == builder.getRequirePresentSupport() &&
              computeFound == builder.getRequireComputeSupport())) {
            return false;
        }
        return true;
    }
}