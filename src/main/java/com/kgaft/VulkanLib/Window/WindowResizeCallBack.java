package com.kgaft.VulkanLib.Window;

import com.kgaft.VulkanLib.Utils.VkErrorException;

import java.lang.instrument.IllegalClassFormatException;

public interface WindowResizeCallBack {
    void resized(int newWidth, int newHeight) throws IllegalClassFormatException, VkErrorException;
}