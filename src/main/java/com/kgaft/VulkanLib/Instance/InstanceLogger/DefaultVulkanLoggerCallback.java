package com.kgaft.VulkanLib.Instance.InstanceLogger;

import java.io.PrintStream;
import java.util.Date;

public class DefaultVulkanLoggerCallback implements IVulkanLoggerCallback{
    @Override
    public void messageRaw(int messageSeverity, int messageType, long pCallbackData, long pUserData) {

    }

    @Override
    public void translatedMessage(String severity, String type, String message, boolean isError) {
        PrintStream output = isError?System.err:System.out;
        String outputMessage = new Date()+" VULKAN "+type+" ["+severity+"] "+message;
        output.println(outputMessage);
    }

    @Override
    public VulkanLoggerCallbackType getCallbackType() {
        return VulkanLoggerCallbackType.TRANSLATED_VULKAN_DEFS;
    }
}