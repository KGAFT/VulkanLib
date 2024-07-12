package com.kgaft.VulkanLib.Utils;

import org.lwjgl.system.MemoryUtil;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class StringByteBuffer {
    private ByteBuffer buffer;
    private String data;

    public StringByteBuffer(String data) {
        this.data = data;
        buffer = MemoryUtil.memUTF8Safe(data, true);
    }

    public ByteBuffer getDataBuffer() {
        return buffer;
    }

    public String getData() {
        return data;
    }
}
