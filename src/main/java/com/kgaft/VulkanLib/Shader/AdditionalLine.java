package com.kgaft.VulkanLib.Shader;

public class AdditionalLine {
    private String line;
    private int index;

    public AdditionalLine(String line, int index) {
        this.line = line;
        this.index = index;
    }

    public AdditionalLine() {
    }

    public String getLine() {
        return line;
    }

    public void setLine(String line) {
        this.line = line;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }
}
