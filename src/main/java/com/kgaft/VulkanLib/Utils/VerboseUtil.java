package com.kgaft.VulkanLib.Utils;

import org.lwjgl.vulkan.*;

import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Optional;

public class VerboseUtil {
    public static void printVkErrorToString(int error){
        if(error!=0){
            System.err.println(vkErrorToString(error));
        }
    }
    public static String vkErrorToString(int error){
        if(error!=0){
            Optional<Field> errorField = Arrays.stream(VK13.class.getFields()).filter(element -> {
                try {
                    return element.getInt(null)==error;
                } catch (Exception e) {
                    return false;
                }
            }).findFirst();
            if(errorField.isEmpty()){
                errorField = Arrays.stream(KHRSurface.class.getFields()).filter(element -> {
                    try {
                        return element.getInt(null)==error;
                    } catch (Exception e) {
                        return false;
                    }
                }).findFirst();
            }
            if(errorField.isEmpty()){
                errorField = Arrays.stream(KHRSwapchain.class.getFields()).filter(element -> {
                    try {
                        return element.getInt(null)==error;
                    } catch (Exception e) {
                        return false;
                    }
                }).findFirst();
            }
            if(errorField.isEmpty()){
                errorField = Arrays.stream(EXTDebugUtils.class.getFields()).filter(element -> {
                    try {
                        return element.getInt(null)==error;
                    } catch (Exception e) {
                        return false;
                    }
                }).findFirst();
            }
            if(errorField.isEmpty()){
                errorField = Arrays.stream(KHRRayTracingPipeline.class.getFields()).filter(element -> {
                    try {
                        return element.getInt(null)==error;
                    } catch (Exception e) {
                        return false;
                    }
                }).findFirst();
            }
            if(errorField.isEmpty()){
                errorField = Arrays.stream(KHRAccelerationStructure.class.getFields()).filter(element -> {
                    try {
                        return element.getInt(null)==error;
                    } catch (Exception e) {
                        return false;
                    }
                }).findFirst();
            }

            return errorField.map(Field::getName).orElse("cannot find exception");
        }
        return "";
    }
}
