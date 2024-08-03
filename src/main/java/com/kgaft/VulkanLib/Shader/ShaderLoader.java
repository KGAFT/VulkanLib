package com.kgaft.VulkanLib.Shader;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.*;
import static org.lwjgl.util.shaderc.Shaderc.*;
import org.lwjgl.vulkan.VkPipelineShaderStageCreateInfo;
import org.lwjgl.vulkan.VkShaderModuleCreateInfo;

import java.io.File;
import java.io.IOException;
import java.lang.instrument.IllegalClassFormatException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.vulkan.VK13.*;
import static org.lwjgl.vulkan.EXTMeshShader.VK_SHADER_STAGE_MESH_BIT_EXT;
import static org.lwjgl.vulkan.KHRRayTracingPipeline.*;

public class ShaderLoader {
    private static boolean initialized = false;
    private static List<String> includeDirectories = new ArrayList<>();

    public static Shader createShader(LogicalDevice device, List<ShaderCreateInfo> shaderInfos) throws IllegalClassFormatException, IOException {
        shaderInfos = sortRayTracingShaders(shaderInfos);
        LwjglObject<VkPipelineShaderStageCreateInfo.Buffer> stages = new LwjglObject<>(VkPipelineShaderStageCreateInfo.class, VkPipelineShaderStageCreateInfo.Buffer.class, shaderInfos.size());
        for (int i = 0; i < shaderInfos.size(); i++) {
            ByteBuffer pName = new StringByteBuffer("main").getDataBuffer();
            stages.get().get(i).sType$Default();
            stages.get().get(i).module(createShaderModule(device, shaderInfos.get(i)));
            stages.get().get(i).stage(shaderInfos.get(i).shaderStage);
            stages.get().get(i).pName(pName);
        }
        return new Shader(stages, device);
    }


    public static Shader createShaderParallel(LogicalDevice device, List<ShaderCreateInfo> shaderInfos, int jobCount) throws IllegalClassFormatException, InterruptedException {
        ThreadPool threadPool = new ThreadPool(jobCount);
        shaderInfos = sortRayTracingShaders(shaderInfos);
        LwjglObject<VkPipelineShaderStageCreateInfo.Buffer> stages = new LwjglObject<>(VkPipelineShaderStageCreateInfo.class, VkPipelineShaderStageCreateInfo.Buffer.class, shaderInfos.size());
        long[] shaderModules = new long[shaderInfos.size()];
        for (int i = 0; i < shaderInfos.size(); i++) {
            threadPool.addTask(new ShaderTask(device, shaderInfos.get(i), i, shaderModules));
        }
        threadPool.executeTasks();
        threadPool.waitForFinish();

        for (int i = 0; i < shaderInfos.size(); i++) {
            stages.get().get(i).sType$Default();
            stages.get().get(i).module(shaderModules[i]);
            ByteBuffer pName = new StringByteBuffer("main").getDataBuffer();
            stages.get().get(i).stage(shaderInfos.get(i).shaderStage);
            stages.get().get(i).pName(pName);
        }
        return new Shader(stages, device);
    }

    static class ShaderTask implements Runnable{
        private ShaderCreateInfo createInfo;
        private int index;
        private long[] output;
        private LogicalDevice device;
        public ShaderTask(LogicalDevice device, ShaderCreateInfo createInfo, int index, long[] output) {
            this.createInfo = createInfo;
            this.index = index;
            this.output = output;
            this.device = device;
        }

        @Override
        public void run() {
            try {
                output[index] = createShaderModule(device, createInfo);
            } catch (IOException | IllegalClassFormatException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private static List<ShaderCreateInfo> sortRayTracingShaders(List<ShaderCreateInfo> createInfos){
        List<ShaderCreateInfo> newStages = new ArrayList<>();
        List<ShaderCreateInfo> rayGenGroup = new ArrayList<>();
        List<ShaderCreateInfo> hitGroup = new ArrayList<>();
        List<ShaderCreateInfo> missGroup = new ArrayList<>();
        List<ShaderCreateInfo> intersectionGroup = new ArrayList<>();
        createInfos.forEach(element->{
            switch (element.shaderStage){
                case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
                    rayGenGroup.add(element);
                    break;
                case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                    hitGroup.add(element);
                    break;
                case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                    hitGroup.add(element);
                    break;
                case VK_SHADER_STAGE_MISS_BIT_KHR:
                    missGroup.add(element);
                    break;
                case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                    intersectionGroup.add(element);
                    break;
                default:
                    newStages.add(element);
                    break;
            }
        });
        rayGenGroup.forEach(newStages::add);
        hitGroup.forEach(newStages::add);
        missGroup.forEach(newStages::add);
        intersectionGroup.forEach(newStages::add);
        return newStages;
    }

    public static List<String> getIncludeDirectories() {
        return includeDirectories;
    }

    public static void setIncludeDirectories(List<String> includeDirectories) {
        ShaderLoader.includeDirectories = includeDirectories;
    }

    public static long createShaderModule(LogicalDevice device, ShaderCreateInfo shaderInfo) throws IOException, IllegalClassFormatException {
        ByteBuffer shaderBinary = null;
        switch (shaderInfo.fileType){
            case SRC_FILE:
                shaderBinary = compileShader(FileReader.readFileText(shaderInfo.pathToFile), new File(shaderInfo.pathToFile).getName(), shaderStageToShaderType(shaderInfo.shaderStage));
                break;
            case BINARY_FILE:
                shaderBinary = FileReader.readFileBinary(shaderInfo.pathToFile);
                break;
        }
        LwjglObject<VkShaderModuleCreateInfo> createInfo = new LwjglObject<>(VkShaderModuleCreateInfo.class);
        createInfo.get().pCode(shaderBinary);
        long[] res = new long[1];
        vkCreateShaderModule(device.getDevice(), createInfo.get(), null, res);
        return res[0];
    }



    public static ByteBuffer compileShader(String content, String fileName, int shaderType) throws IOException {
        long compiler = shaderc_compiler_initialize();

        if(compiler == 0) {
            throw new RuntimeException("Failed to create shader compiler");
        }
        content = Includer.processIncludes(content, includeDirectories);
        ByteBuffer source = new StringByteBuffer(content).getDataBuffer();
        ByteBuffer filename = new StringByteBuffer(fileName).getDataBuffer();
        ByteBuffer mainName = new StringByteBuffer("main").getDataBuffer();
        long options = shaderc_compile_options_initialize();
        shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_6);
        long result = shaderc_compile_into_spv(compiler, source, shaderType, filename, mainName, options);;

        if(result == 0) {
            throw new RuntimeException("Failed to compile shader " + filename + " into SPIR-V");
        }

        if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
            System.err.println("Shader code: ");

            int i = 0;
            int line = 0;
            System.err.print(line+" ");
            while(i<content.length()) {
                System.err.print(content.charAt(i));
                if(content.charAt(i)=='\n') {
                    line++;
                    System.err.print(line+" ");
                }
                i++;
            }
            throw new RuntimeException("Failed to compile shader " + filename + "into SPIR-V:\n " + shaderc_result_get_error_message(result));
        }

        shaderc_compiler_release(compiler);

        return shaderc_result_get_bytes(result);
    }


    private static int shaderStageToShaderType(int shaderStage){
        switch (shaderStage){
            case VK_SHADER_STAGE_COMPUTE_BIT:
                return shaderc_compute_shader;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                return shaderc_fragment_shader;
            case VK_SHADER_STAGE_VERTEX_BIT:
                return shaderc_vertex_shader;
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                return shaderc_geometry_shader;
            case VK_SHADER_STAGE_MESH_BIT_EXT:
                return shaderc_mesh_shader;
            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
                return shaderc_raygen_shader;
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                return shaderc_closesthit_shader;
            case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                return shaderc_anyhit_shader;
            case VK_SHADER_STAGE_MISS_BIT_KHR:
                return shaderc_miss_shader;
            case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                return shaderc_intersection_shader;
        }
        return -1;
    }

}
