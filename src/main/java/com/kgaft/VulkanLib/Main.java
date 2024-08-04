package com.kgaft.VulkanLib;

import com.kgaft.VulkanLib.Device.DeviceBuilder;
import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Device.PhysicalDevice.DeviceSuitability;
import com.kgaft.VulkanLib.Device.PhysicalDevice.DeviceSuitabilityResults;
import com.kgaft.VulkanLib.Device.SwapChain;
import com.kgaft.VulkanLib.Instance.Instance;
import com.kgaft.VulkanLib.Instance.InstanceBuilder;
import com.kgaft.VulkanLib.Instance.InstanceLogger.DefaultVulkanFileLoggerCallback;
import com.kgaft.VulkanLib.Instance.InstanceLogger.DefaultVulkanLoggerCallback;
import com.kgaft.VulkanLib.Shader.Shader;
import com.kgaft.VulkanLib.Shader.ShaderCreateInfo;
import com.kgaft.VulkanLib.Shader.ShaderFileType;
import com.kgaft.VulkanLib.Shader.ShaderLoader;
import com.kgaft.VulkanLib.Device.PhysicalDevice.PhysicalDevice;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import com.kgaft.VulkanLib.Window.Window;

import org.lwjgl.vulkan.KHRRayTracingPipeline;

import java.io.IOException;
import java.lang.instrument.IllegalClassFormatException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


public class Main {
    public static void main(String[] args) throws IllegalClassFormatException, VkErrorException, IOException, InterruptedException {

        Window.prepareWindow(1280, 720, "Vulan lib development", true);
        Window window = Window.getWindow();
        InstanceBuilder instanceBuilder = new InstanceBuilder();
        instanceBuilder.presetForDebug();
        instanceBuilder.setApplicationName("VulkanLib testing app");
        instanceBuilder.setEngineName("VulkanLib testing engine");
        instanceBuilder.setApplicationVersion(1,0,0);
        instanceBuilder.setEngineVersion(1,0,0);
        instanceBuilder.presetForPresent();
        instanceBuilder.addStartingVulkanLoggerCallback(new DefaultVulkanLoggerCallback());
        instanceBuilder.addStartingVulkanLoggerCallback(new DefaultVulkanFileLoggerCallback());
        Instance instance = new Instance(instanceBuilder);
        DeviceBuilder builder = new DeviceBuilder();
        builder.requestGraphicSupport();
        builder.requestPresentSupport(window.getSurface(instance.getInstance()));
        builder.requestComputeSupport();
        builder.requestRayTracingSupport();
        HashMap<PhysicalDevice, DeviceSuitabilityResults> supportedDevices = new HashMap<>();
        PhysicalDevice.getPhysicalDevices(instance).forEach(element->{
            try {
                DeviceSuitabilityResults results = DeviceSuitability.isDeviceSuitable(builder, element);
                if(results!=null){
                    supportedDevices.put(element, results);
                }
            } catch (VkErrorException e) {
                throw new RuntimeException(e);
            }
        });
        supportedDevices.forEach((element, val)->{
            System.out.println(element.getProperties().get().deviceNameString());
        });
        LogicalDevice device = new LogicalDevice(instance, PhysicalDevice.getPhysicalDevices(instance).get(0), builder, supportedDevices.get(PhysicalDevice.getPhysicalDevices(instance).get(0)));
        SwapChain swapChain = new SwapChain(device, window.getSurface(instance.getInstance()), window.getWidth(), window.getHeight(), true);
        window.addResizeCallBack((swapChain::recreate));
        List<ShaderCreateInfo> createInfoList = new ArrayList<>();
        createInfoList.add(new ShaderCreateInfo("raygen.glsl", ShaderFileType.SRC_FILE, KHRRayTracingPipeline.VK_SHADER_STAGE_RAYGEN_BIT_KHR, new ArrayList<>()));
        createInfoList.add(new ShaderCreateInfo("closehit.glsl", ShaderFileType.SRC_FILE, KHRRayTracingPipeline.VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, new ArrayList<>()));
        createInfoList.add(new ShaderCreateInfo("miss.glsl", ShaderFileType.SRC_FILE, KHRRayTracingPipeline.VK_SHADER_STAGE_MISS_BIT_KHR, new ArrayList<>()));
        Shader shader = ShaderLoader.createShaderParallel(device, createInfoList, 3);
        while(window.isWindowActive()){
            window.postEvents();
        }


    }
}