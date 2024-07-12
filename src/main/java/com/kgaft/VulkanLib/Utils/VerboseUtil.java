package com.kgaft.VulkanLib.Utils;

import org.lwjgl.vulkan.VK13;

import java.lang.reflect.Field;
import java.util.Arrays;

public class VerboseUtil {
    public static void printVkErrorToString(int error){
        if(error!=0){
            Field errorField = Arrays.stream(VK13.class.getFields()).filter(element -> {
                try {
                    return element.getInt(null)==error;
                } catch (Exception e) {
                    return false;
                }
            }).findFirst().get();
            System.err.println(errorField.getName());
        }
    }
}
