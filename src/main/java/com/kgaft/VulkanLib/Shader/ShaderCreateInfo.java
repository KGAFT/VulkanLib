package com.kgaft.VulkanLib.Shader;

import java.util.ArrayList;
import java.util.List;

public class ShaderCreateInfo {
    public String pathToFile;
    public ShaderFileType fileType;
    public int shaderStage;
    public List<AdditionalLine> additionalLines = new ArrayList<>();
}
