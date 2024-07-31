package com.kgaft.VulkanLib.Shader;

import graphics.scenery.spirvcrossj.*;

import java.io.IOException;

public class ShaderLoader {
    private static boolean initialized = false;
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
    public static IntVec compileShader(String content, int shaderType) throws IOException {

        final TProgram program = new TProgram();
        final TShader shader = new TShader(shaderType);
        final SWIGTYPE_p_TBuiltInResource resources = libspirvcrossj.getDefaultTBuiltInResource();
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

}
