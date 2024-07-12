package com.kgaft.VulkanLib.Instance.InstanceLogger;

import com.kgaft.VulkanLib.Utils.DestroyableObject;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.VerboseUtil;
import org.lwjgl.vulkan.*;

import java.io.PrintStream;
import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;
import java.util.List;

import static com.kgaft.VulkanLib.Instance.InstanceLogger.VulkanLoggerCallbackType.*;
import static org.lwjgl.vulkan.EXTDebugUtils.*;
import static org.lwjgl.vulkan.VK10.VK_FALSE;
import static org.lwjgl.vulkan.VK10.VK_SUCCESS;

public class InstanceLogger extends DestroyableObject implements VkDebugUtilsMessengerCallbackEXTI {
    public InstanceLogger(){
        try {
            createInfo = new LwjglObject<VkDebugUtilsMessengerCreateInfoEXT>(VkDebugUtilsMessengerCreateInfoEXT.class);
            createInfo.get().sType$Default();
            createInfo.get().messageSeverity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT);
            createInfo.get().messageType(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
            createInfo.get().pfnUserCallback(this);

        } catch (IllegalClassFormatException e) {
            e.printStackTrace();
        }
    }
    private LwjglObject<VkDebugUtilsMessengerCreateInfoEXT> createInfo;
    private long handle;
    private VkInstance instance;
    private List<IVulkanLoggerCallback> callbacks = new ArrayList<>();

    public void describeLogger(VkInstanceCreateInfo createInfo){
        createInfo.pNext(this.createInfo.get());
    }

    public void init(VkInstance instance){
        long[] res = new long[1];
        int status = vkCreateDebugUtilsMessengerEXT(instance, createInfo.get(), null, res);
        if(status!=VK_SUCCESS){
            VerboseUtil.printVkErrorToString(status);
            throw new RuntimeException("Failed to create vulkan debugger");
        }
        this.handle = res[0];
        this.instance = instance;
    }

    @Override
    public int invoke(int messageSeverity, int messageType, long pCallbackData, long pUserData) {
        VkDebugUtilsMessengerCallbackDataEXT callbackData = VkDebugUtilsMessengerCallbackDataEXT.create(pCallbackData);
        if (callbacks.size() == 0) {

            PrintStream outputStream = messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? System.err : System.out;
            outputStream.println("Vulkan: " + callbackData.pMessageString());

        } else {
            StringBuilder textSeverity = new StringBuilder();
            StringBuilder textType = new StringBuilder();
            callbacks.forEach(callback -> {
                switch (callback.getCallbackType()) {
                    case RAW_VULKAN_DEFS:
                        callback.messageRaw(messageSeverity, messageType, pCallbackData, pUserData);
                        break;
                    case TRANSLATED_VULKAN_DEFS:
                        translateDebugMessageData(textSeverity, textType, messageSeverity, messageType);
                        callback.translatedMessage(textSeverity.toString(), textType.toString(), callbackData.pMessageString(), messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
                        break;
                    case BOTH_VULKAN_DEFS:
                        callback.messageRaw(messageSeverity, messageType, pCallbackData, pUserData);
                        translateDebugMessageData(textSeverity, textType, messageSeverity, messageType);
                        callback.translatedMessage(textSeverity.toString(), textType.toString(), callbackData.pMessageString(), messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
                        break;
                }
            });
        }
        return VK_FALSE;
    }

    public void registerCallback(IVulkanLoggerCallback callback) {
        callbacks.add(callback);
    }

    public void removeCallback(IVulkanLoggerCallback callback) {
        callbacks.remove(callback);
    }

    private void translateDebugMessageData(StringBuilder textSeverity, StringBuilder textType, int messageSeverity, int messageType) {
        if (textSeverity.length() == 0 && textType.length() == 0) {
            textSeverity.append(translateSeverity(messageSeverity));
            textType.append(translateType(messageType));
        }
    }

    private String translateSeverity(int severity) {
        String res;
        switch (severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                res = "VERBOSE";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                res = "ERROR";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                res = "WARNING";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                res = "INFO";
                break;
            default:
                res = "UNDEFINED";
                break;
        }
        return res;
    }

    private String translateType(int type) {
        String res = "";
        switch (type) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                res = "GENERAL";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                res = "PERFORMANCE";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                res = "VALIDATION";
                break;
            default:
                res = "UNDEFINED";
                break;
        }
        return res;
    }

    @Override
    public void destroy() {
        this.destroyed = true;
        vkDestroyDebugUtilsMessengerEXT(instance, handle, null);
    }

}
