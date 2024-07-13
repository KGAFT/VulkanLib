package com.kgaft.VulkanLib.Utils;

public class VkErrorException extends Throwable{
    public VkErrorException(String text, int error) {
        super(VerboseUtil.vkErrorToString(error));
    }
}
