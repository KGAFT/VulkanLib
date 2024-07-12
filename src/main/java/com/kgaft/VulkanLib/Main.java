package com.kgaft.VulkanLib;

import com.kgaft.VulkanLib.Instance.Instance;
import com.kgaft.VulkanLib.Instance.InstanceBuilder;
import com.kgaft.VulkanLib.Window.Window;

import java.lang.instrument.IllegalClassFormatException;

public class Main {
    public static void main(String[] args) throws IllegalClassFormatException {
        Window.prepareWindow(1280, 720, "Vulan lib development", true);
        Window window = Window.getWindow();
        InstanceBuilder instanceBuilder = new InstanceBuilder();
        instanceBuilder.presetForDebug();
        instanceBuilder.setApplicationName("VulkanLib testing app");
        instanceBuilder.setEngineName("VulkanLib testing engine");
        instanceBuilder.setApplicationVersion(1,0,0);
        instanceBuilder.setEngineVersion(1,0,0);
        instanceBuilder.presetForPresent();
        Instance instance = new Instance(instanceBuilder);
    }
}