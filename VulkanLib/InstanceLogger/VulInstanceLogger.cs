using Silk.NET.Vulkan;
using System.Runtime.InteropServices;
using System.Text;
using Silk.NET.Vulkan.Extensions.EXT;
using Silk.NET.Vulkan.Extensions.KHR;
using VulkanLib.ObjectManagement;

namespace VulkanLib.InstanceLogger;

public unsafe class VulInstanceLogger :  DestroyableObject
{
    public void describeLogger(ref DebugUtilsMessengerCreateInfoEXT createInfo)
    {
        createInfo.SType = StructureType.DebugUtilsMessengerCreateInfoExt;
        createInfo.MessageType = DebugUtilsMessageTypeFlagsEXT.GeneralBitExt | DebugUtilsMessageTypeFlagsEXT.PerformanceBitExt | DebugUtilsMessageTypeFlagsEXT.ValidationBitExt | DebugUtilsMessageTypeFlagsEXT.DeviceAddressBindingBitExt;
        createInfo.MessageSeverity = DebugUtilsMessageSeverityFlagsEXT.VerboseBitExt |
                                     DebugUtilsMessageSeverityFlagsEXT.WarningBitExt |
                                     DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt |
                                     DebugUtilsMessageSeverityFlagsEXT.InfoBitExt;
        createInfo.PfnUserCallback = (DebugUtilsMessengerCallbackFunctionEXT)debugCallback;
    }
    public VulInstanceLogger()
    {
        
    }

    private ExtDebugUtils debugUtils;
    private DebugUtilsMessengerEXT messenger;
    private List<IVulInstanceLoggerCallback> callbacks = new ();
    private Instance instance;
    public void initialize(Instance instance, ExtDebugUtils debugUtils)
    {
        this.debugUtils = debugUtils;
        this.instance = instance;
        DebugUtilsMessengerCreateInfoEXT createInfo = new DebugUtilsMessengerCreateInfoEXT();
        describeLogger(ref createInfo);
        if(debugUtils.CreateDebugUtilsMessenger(instance, &createInfo, null, out messenger) != Result.Success)
        {
            throw new Exception("Failed to create debug messenger instance!");
        }
    }
    private uint debugCallback(DebugUtilsMessageSeverityFlagsEXT messageSeverity, DebugUtilsMessageTypeFlagsEXT messageType, DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (callbacks.Capacity == 0) {

            var outputStream = messageSeverity == DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt ? Console.Error : Console.Out;
            outputStream.WriteLine("Vulkan: " + Marshal.PtrToStringAnsi((nint)pCallbackData->PMessage));

        } else {
            StringBuilder textSeverity = new StringBuilder();
            StringBuilder textType = new StringBuilder();
            foreach (var callback in callbacks)
            {
                switch (callback.getCallbackType()) {
                    case VulkanLoggerCallbackType.RAW_VULKAN_DEFS:
                        callback.messageRaw(messageSeverity, messageType, pCallbackData, pUserData);
                        break;
                    case VulkanLoggerCallbackType.TRANSLATED_VULKAN_DEFS:
                        translateDebugMessageData(textSeverity, textType, messageSeverity, messageType);
                        callback.translatedMessage(textSeverity.ToString(), textType.ToString(), Marshal.PtrToStringAnsi((nint)pCallbackData->PMessage), messageSeverity == DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt);
                        break;
                    case VulkanLoggerCallbackType.BOTH_VULKAN_DEFS:
                        callback.messageRaw(messageSeverity, messageType, pCallbackData, pUserData);
                        translateDebugMessageData(textSeverity, textType, messageSeverity, messageType);
                        callback.translatedMessage(textSeverity.ToString(), textType.ToString(), Marshal.PtrToStringAnsi((nint)pCallbackData->PMessage), messageSeverity == DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt);
                        break;
                }
            }
        }
        return Vk.False;
    }
    public void registerCallback(IVulInstanceLoggerCallback callback) {
        callbacks.Add(callback);
    }

    public void removeCallback(IVulInstanceLoggerCallback callback) {
        callbacks.Remove(callback);
    }

    
    public override void destroy()
    {
        base.destroy();
        debugUtils.DestroyDebugUtilsMessenger(instance, messenger, null);
    }
    
    private void translateDebugMessageData(StringBuilder textSeverity, StringBuilder textType, DebugUtilsMessageSeverityFlagsEXT messageSeverity, DebugUtilsMessageTypeFlagsEXT messageType) {
        if (textSeverity.Length == 0 && textType.Length == 0) {
            textSeverity.Append(translateSeverity(messageSeverity));
            textType.Append(translateType(messageType));
        }
    }
    
    private String translateSeverity(DebugUtilsMessageSeverityFlagsEXT severity) {
        String res;
        switch (severity) {
            case DebugUtilsMessageSeverityFlagsEXT.VerboseBitExt:
                res = "VERBOSE";
                break;
            case DebugUtilsMessageSeverityFlagsEXT.ErrorBitExt:
                res = "ERROR";
                break;
            case DebugUtilsMessageSeverityFlagsEXT.WarningBitExt:
                res = "WARNING";
                break;
            case DebugUtilsMessageSeverityFlagsEXT.InfoBitExt:
                res = "INFO";
                break;
            default:
                res = "UNDEFINED";
                break;
        }
        return res;
    }

    private String translateType(DebugUtilsMessageTypeFlagsEXT type) {
        String res = "";
        switch (type) {
            case DebugUtilsMessageTypeFlagsEXT.GeneralBitExt:
                res = "GENERAL";
                break;
            case DebugUtilsMessageTypeFlagsEXT.PerformanceBitExt:
                res = "PERFORMANCE";
                break;
            case DebugUtilsMessageTypeFlagsEXT.ValidationBitExt:
                res = "VALIDATION";
                break;
            case DebugUtilsMessageTypeFlagsEXT.DeviceAddressBindingBitExt:
                res = "DEVICE_ADDRESS_BINDING";
                break;
            default:
                res = "UNDEFINED";
                break;
        }
        return res;
    }
}