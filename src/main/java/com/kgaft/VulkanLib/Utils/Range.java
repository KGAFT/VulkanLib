package com.kgaft.VulkanLib.Utils;

public class Range {
    private double begin;
    private double end;

    public Range(double begin, double end) {
        this.begin = begin;
        this.end = end;
    }
    public Range(){

    }

    public double getBegin() {
        return begin;
    }

    public void setBegin(double begin) {
        this.begin = begin;
    }

    public double getEnd() {
        return end;
    }

    public void setEnd(double end) {
        this.end = end;
    }
}
