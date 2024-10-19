using Silk.NET.Vulkan;

namespace VulkanLib.InstanceLogger;

public unsafe interface IVulInstanceLoggerCallback
{
    void messageRaw(DebugUtilsMessageSeverityFlagsEXT messageSeverity, DebugUtilsMessageTypeFlagsEXT messageTypes, DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    void translatedMessage(String severity, String type, String message, bool isError);
    VulkanLoggerCallbackType getCallbackType();
}

public enum VulkanLoggerCallbackType
{
    RAW_VULKAN_DEFS,
    TRANSLATED_VULKAN_DEFS,
    BOTH_VULKAN_DEFS
}