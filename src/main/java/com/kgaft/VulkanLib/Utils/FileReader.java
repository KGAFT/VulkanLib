package com.kgaft.VulkanLib.Utils;

import graphics.scenery.spirvcrossj.IntVec;
import org.lwjgl.vulkan.VkShaderModuleCreateInfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.nio.ByteBuffer;

public class FileReader {
    public static String readFileText(String path) {
        if(new File(path).exists()){
            StringBuilder content = new StringBuilder();
            try{
                BufferedReader reader = new BufferedReader(new java.io.FileReader(path));
                reader.lines().forEach(element->{
                    content.append(element);
                    content.append("\n");
                });
                return content.toString();
            }catch (Exception e){
                e.printStackTrace();
                return null;
            }

        }
        return null;
    }
    public static ByteBuffer readFileBinary(String path){
        File file = new File(path);
        if(file.exists()){
            try {
                FileInputStream fis = new FileInputStream(file);
                byte[] content = fis.readAllBytes();
                return ByteBuffer.wrap(content);
            }catch (Exception e) {
                e.printStackTrace();
                return null;
            }
        }
        return null;
    }
}
