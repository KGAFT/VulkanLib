package com.kgaft.VulkanLib.Instance.InstanceLogger;

import java.io.*;
import java.util.Date;

public class DefaultVulkanFileLoggerCallback implements IVulkanLoggerCallback{
    public DefaultVulkanFileLoggerCallback(String fileName) throws FileNotFoundException {
        fos = new FileOutputStream(new File(fileName));
    }
    public DefaultVulkanFileLoggerCallback() throws FileNotFoundException {
        fos = new FileOutputStream("log.txt");
    }
    private FileOutputStream fos;
    @Override
    public void messageRaw(int messageSeverity, int messageType, long pCallbackData, long pUserData) {

    }

    @Override
    public void translatedMessage(String severity, String type, String message, boolean isError) {

        String outputMessage = new Date()+" VULKAN "+type+" ["+severity+"] "+message+"\n";
        try {
            fos.write(outputMessage.getBytes());
            fos.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public VulkanLoggerCallbackType getCallbackType() {
        return VulkanLoggerCallbackType.TRANSLATED_VULKAN_DEFS;
    }
}