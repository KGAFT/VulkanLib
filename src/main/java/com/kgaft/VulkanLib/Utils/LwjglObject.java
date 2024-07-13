package com.kgaft.VulkanLib.Utils;

import org.lwjgl.system.MemoryStack;

import java.lang.instrument.IllegalClassFormatException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Optional;

public class LwjglObject<T> {
    private final T baseObject;
    private Class<T> typeParameterClass;
    private Class baseTypeClass;
    private final Method freeMethod;

    public LwjglObject(Class<T> typeParameterClass) throws IllegalClassFormatException {
        this.typeParameterClass = typeParameterClass;
        Optional<Method> optAllocateMethod = Arrays.stream(typeParameterClass.getMethods())
                .filter(method -> method.getName().equals("calloc"))
                .filter(method -> method.getParameters().length == 0)
                .filter(method -> method.getReturnType() == typeParameterClass)
                .findFirst();

        Optional<Method> freeMethod = Arrays.stream(typeParameterClass.getMethods())
                .filter(method -> method.getName().equals("free"))
                .filter(method -> method.getParameters().length == 0).findFirst();
        if (optAllocateMethod.isEmpty() || freeMethod.isEmpty()) {
            throw new IllegalClassFormatException("It is not lwjgl allocatable object");
        }
        Method allocateMethod = optAllocateMethod.get();
        try {
            baseObject = (T) allocateMethod.invoke(null);
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new RuntimeException(e);
        }
        this.freeMethod = freeMethod.get();
    }

    public LwjglObject(Class baseClass, Class<T> bufferClass, int capacity) throws IllegalClassFormatException {
        this.typeParameterClass = bufferClass;
        this.baseTypeClass = baseClass;
        Optional<Method> optAllocateMethod = Arrays.stream(baseTypeClass.getMethods()).filter(method -> method.getName().equals("calloc"))
                .filter(method -> method.getParameters().length == 1)
                .filter(method -> method.getParameters()[0].getType().equals(int.class)).findFirst();
        Optional<Method> freeMethod = Arrays.stream(typeParameterClass.getMethods())
                .filter(method -> method.getName().equals("free"))
                .filter(method -> method.getParameters().length == 0).findFirst();
        if (optAllocateMethod.isEmpty() || freeMethod.isEmpty()) {
            throw new IllegalClassFormatException("It is not lwjgl buffer allocatable object");
        }
        Method allocateMethod = optAllocateMethod.get();
        try {
            baseObject = (T) allocateMethod.invoke(null, capacity);
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new RuntimeException(e);
        }
        this.freeMethod = freeMethod.get();

    }

    public T get() {
        return baseObject;
    }

    @Override
    protected void finalize() throws Throwable {
        freeMethod.invoke(baseObject);
        super.finalize();
    }
}
