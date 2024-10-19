using Silk.NET.Vulkan;

namespace VulkanLib.InstanceLogger;

public class DefaultVulkanLoggerCallback : IVulInstanceLoggerCallback
{
    public unsafe void messageRaw(DebugUtilsMessageSeverityFlagsEXT messageSeverity, DebugUtilsMessageTypeFlagsEXT messageTypes,
        DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        throw new NotImplementedException();
    }

    public void translatedMessage(string severity, string type, string message, bool isError)
    {
        var output = isError?Console.Error:Console.Out;
        String outputMessage = DateTime.Now+" VULKAN "+type+" ["+severity+"] "+message;
        output.WriteLine(outputMessage);
    }

    public VulkanLoggerCallbackType getCallbackType()
    {
        return VulkanLoggerCallbackType.TRANSLATED_VULKAN_DEFS;
    }
}