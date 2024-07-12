package com.kgaft.VulkanLib.Instance.InstanceLogger;

public interface IVulkanLoggerCallback {
    void messageRaw(int messageSeverity, int messageType, long pCallbackData, long pUserData);
    void translatedMessage(String severity, String type, String message, boolean isError);
    VulkanLoggerCallbackType getCallbackType();
}
