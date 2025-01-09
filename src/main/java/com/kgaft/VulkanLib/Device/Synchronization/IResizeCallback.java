package com.kgaft.VulkanLib.Device.Synchronization;

import com.kgaft.VulkanLib.Utils.VkErrorException;

import java.lang.instrument.IllegalClassFormatException;

public interface IResizeCallback {
    void resized(int newWidth, int newHeight) throws IllegalClassFormatException, VkErrorException;
}