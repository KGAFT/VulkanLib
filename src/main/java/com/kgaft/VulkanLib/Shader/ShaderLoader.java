package com.kgaft.VulkanLib.Shader;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.FileReader;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import com.kgaft.VulkanLib.Utils.SeriesObject;
import com.kgaft.VulkanLib.Utils.StringByteBuffer;
import graphics.scenery.spirvcrossj.*;
import org.lwjgl.vulkan.VkPipelineShaderStageCreateInfo;
import org.lwjgl.vulkan.VkShaderModuleCreateInfo;

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
    private static SeriesObject<VkShaderModuleCreateInfo> createInfos;

    static {
        try {
            createInfos = new SeriesObject<>(VkShaderModuleCreateInfo.class);
        } catch (IllegalClassFormatException e) {
            throw new RuntimeException(e);
        }
    }

    public static void init(){
        if(!initialized){
            try {
                Loader.loadNatives();
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            if (!libspirvcrossj.initializeProcess()) {
                throw new RuntimeException("glslang failed to initialize.");
            }
            initialized = true;
        }
    }


    public Shader createShader(LogicalDevice device, List<ShaderCreateInfo> shaderInfos) throws IllegalClassFormatException, IOException {
        shaderInfos = sortRayTracingShaders(shaderInfos);
        LwjglObject<VkPipelineShaderStageCreateInfo.Buffer> stages = new LwjglObject<>(VkPipelineShaderStageCreateInfo.class, VkPipelineShaderStageCreateInfo.Buffer.class, shaderInfos.size());
        ByteBuffer pName = new StringByteBuffer("main").getDataBuffer();
        for (int i = 0; i < shaderInfos.size(); i++) {
            stages.get().get(i).sType$Default();
            stages.get().get(i).module(createShaderModule(device, shaderInfos.get(i)));
            stages.get().get(i).stage(shaderInfos.get(i).shaderStage);
            stages.get().get(i).pName(pName);
        }
        return new Shader(stages, device);
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

    public long createShaderModule(LogicalDevice device, ShaderCreateInfo shaderInfo) throws IOException {
        ByteBuffer shaderBinary = null;
        switch (shaderInfo.fileType){
            case SRC_FILE:
                shaderBinary = intVecToByteBuffer(compileShader(FileReader.readFileText(shaderInfo.pathToFile), shaderStageToShaderType(shaderInfo.shaderStage)));
                break;
            case BINARY_FILE:
                shaderBinary = FileReader.readFileBinary(shaderInfo.pathToFile);
                break;
        }
        VkShaderModuleCreateInfo createInfo = createInfos.acquireObject();
        createInfo.pCode(shaderBinary);
        long[] res = new long[1];
        vkCreateShaderModule(device.getDevice(), createInfo, null, res);
        createInfos.releaseObjectInstance(createInfo);
        return res[0];
    }



    public static IntVec compileShader(String content, int shaderType) throws IOException {

        final TProgram program = new TProgram();
        final TShader shader = new TShader(shaderType);
        final SWIGTYPE_p_TBuiltInResource resources = libspirvcrossj.getDefaultTBuiltInResource();

        content = Includer.processIncludes(content, new ArrayList<>());
        shader.setStrings(new String[] { content }, 1);
        shader.setAutoMapBindings(true);
        boolean compileFail = false;
        boolean linkFail = false;
        int messages = EShMessages.EShMsgDefault;
        messages |= EShMessages.EShMsgVulkanRules;
        messages |= EShMessages.EShMsgSpvRules;

        if (!shader.parse(resources, 450, false, messages)) {
            compileFail = true;
        }

        if (compileFail) {
            System.out.println("Info log: " + shader.getInfoLog());
            System.out.println("Debug log: " + shader.getInfoDebugLog());
            throw new RuntimeException("Compilation of content: " + content + " failed");
        }

        program.addShader(shader);

        if (!program.link(EShMessages.EShMsgDefault)) {
            linkFail = true;
        }

        if (!program.mapIO()) {
            linkFail = true;
        }

        if (linkFail) {
            System.err.println(program.getInfoLog());
            System.err.println(program.getInfoDebugLog());

            throw new RuntimeException("Linking of program: " + content + " failed!");
        }

        if (!linkFail && !compileFail) {
            final IntVec spirv = new IntVec();
            libspirvcrossj.glslangToSpv(program.getIntermediate(shaderType), spirv);
            return spirv;
        }
        return null;
    }

    private static ByteBuffer intVecToByteBuffer(IntVec src){
        ByteBuffer res = ByteBuffer.allocate(src.size());
        src.forEach(element -> res.put(element.byteValue()));
        return res;
    }

    private static int shaderStageToShaderType(int shaderStage){
        switch (shaderStage){
            case VK_SHADER_STAGE_COMPUTE_BIT:
                return EShLanguage.EShLangCompute;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                return EShLanguage.EShLangFragment;
            case VK_SHADER_STAGE_VERTEX_BIT:
                return EShLanguage.EShLangVertex;
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                return EShLanguage.EShLangGeometry;
            case VK_SHADER_STAGE_MESH_BIT_EXT:
                return EShLanguage.EShLangMeshNV;
            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
                return EShLanguage.EShLangRayGenNV;
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                return EShLanguage.EShLangClosestHitNV;
            case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                return EShLanguage.EShLangAnyHitNV;
            case VK_SHADER_STAGE_MISS_BIT_KHR:
                return EShLanguage.EShLangMissNV;
            case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                return EShLanguage.EShLangIntersectNV;
        }
        return -1;
    }

}
