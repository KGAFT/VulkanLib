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
import com.kgaft.VulkanLib.Device.PhysicalDevice.PhysicalDevice;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import com.kgaft.VulkanLib.Window.Window;

import java.io.FileNotFoundException;
import java.lang.instrument.IllegalClassFormatException;
import java.util.HashMap;

public class Main {
    public static void main(String[] args) throws IllegalClassFormatException, FileNotFoundException, VkErrorException {
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
        while(window.isWindowActive()){
            window.postEvents();
        }
    }
}