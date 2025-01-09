package com.kgaft.VulkanLib.Utils;

import static org.lwjgl.vulkan.VK10.VK_SUCCESS;

public class VkErrorException extends Throwable{
    public VkErrorException(String text, int error) {
        super(VerboseUtil.vkErrorToString(error));
    }
    public static void checkVkStatus(String errorTextIfNeeded, int status) throws VkErrorException {
        if(status!=VK_SUCCESS){
            throw new VkErrorException(errorTextIfNeeded, status);
        }
    }

    public static void printVkError(String errorTextIfNeeded, int status) throws VkErrorException {
        if(status!=VK_SUCCESS){
            System.err.println(errorTextIfNeeded+VerboseUtil.vkErrorToString(status));
        }
    }
}
