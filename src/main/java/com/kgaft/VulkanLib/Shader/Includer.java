package com.kgaft.VulkanLib.Shader;

import com.kgaft.VulkanLib.Utils.FileReader;
import com.kgaft.VulkanLib.Utils.Range;

import java.util.ArrayList;

public class Includer {
    public static String processIncludes(String content, ArrayList<String> includeDirectories){
        int index = content.lastIndexOf("#include");
        while(index!=-1){
            Range range = new Range();
            range.setBegin(index);
            String currentInclude = getInclude(content, range);
            String includeContent = FileReader.readFileText(currentInclude);
            int i = 0;
            while(includeContent==null && i<includeDirectories.size()){
                includeContent = FileReader.readFileText(includeDirectories.get(i)+"/"+currentInclude);
                i++;
            }
            content = replaceInclude(content, range, includeContent);
            index = content.lastIndexOf("#include");
        }
        return content;
    }

    private static String replaceInclude(String content, Range replaceRange, String includeContent){
        String first = content.substring(0, (int)replaceRange.getBegin());
        String second = content.substring((int)replaceRange.getEnd() + 1);
        return first+includeContent+second;
    }

    private static String getInclude(String content, Range index){
        boolean incBegin = false;
        String currentInclude = "";
        for(int i = (int) index.getBegin(); ; i++){
            char curChar = content.charAt(i);
            if((curChar=='"' && incBegin) || curChar=='>'){
                incBegin = false;
                index.setEnd(i);
                break;
            }
            if(incBegin){
                currentInclude+=curChar;
            }
            if(curChar=='\"' || curChar=='<'){
                incBegin = true;
            }
        }
        return currentInclude;
    }
}
