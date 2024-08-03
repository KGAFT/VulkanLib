package com.kgaft.VulkanLib.Shader;

import java.util.ArrayList;
import java.util.List;

public class ShaderCreateInfo {
    public String pathToFile;
    public ShaderFileType fileType;
    public int shaderStage;
    public List<AdditionalLine> additionalLines = new ArrayList<>();

    public ShaderCreateInfo(String pathToFile, ShaderFileType fileType, int shaderStage, List<AdditionalLine> additionalLines) {
        this.pathToFile = pathToFile;
        this.fileType = fileType;
        this.shaderStage = shaderStage;
        this.additionalLines = additionalLines;
    }

    public ShaderCreateInfo() {
    }
}
