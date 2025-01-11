package com.kgaft.VulkanLib.Utils;

import java.nio.ByteBuffer;

public class MemoryUtils {
    public static void memcpy(ByteBuffer dst, ByteBuffer src, long size) {
        src.limit((int)size);
        dst.put(src);
        src.limit(src.capacity()).rewind();
    }
}
