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
import com.kgaft.VulkanLib.Shader.ShaderLoader;
import com.kgaft.VulkanLib.Device.PhysicalDevice.PhysicalDevice;
import com.kgaft.VulkanLib.Utils.ThreadPool;
import com.kgaft.VulkanLib.Utils.VkErrorException;
import com.kgaft.VulkanLib.Window.Window;

import graphics.scenery.spirvcrossj.EShLanguage;
import graphics.scenery.spirvcrossj.IntVec;
import graphics.scenery.spirvcrossj.Loader;
import graphics.scenery.spirvcrossj.libspirvcrossj;
import org.joml.Matrix4f;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.lang.instrument.IllegalClassFormatException;
import java.nio.FloatBuffer;
import java.util.HashMap;
import java.util.Random;

public class Main {
    public static void main(String[] args) throws IllegalClassFormatException, VkErrorException, IOException, InterruptedException {
        /*
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
        StringBuilder content = new StringBuilder();
        BufferedReader reader = new BufferedReader(new FileReader("content.vert"));
        reader.lines().forEach(element->{
            content.append(element);
            content.append("\n");
        });
        ShaderLoader.init();
        IntVec shaderContent = ShaderLoader.compileShader(content.toString(), EShLanguage.EShLangVertex);

         */
        ThreadPool threadPool = new ThreadPool(28);
        for(int i = 0; i<128; i++){
            threadPool.addTask(()->{
                float[] values = new float[16];
                for (int i1 = 0; i1 < values.length; i1++) {
                    values[i1] = new Random().nextFloat();
                }
                Matrix4f matrix = new Matrix4f(FloatBuffer.wrap(values));
                float determinant = matrix.determinant();
            });
        }
        threadPool.executeTasks();
        threadPool.waitForFinish();
        /*
        while(window.isWindowActive()){
            window.postEvents();
        }

         */
    }
}