package com.kgaft.VulkanLib.Utils;

import java.lang.instrument.IllegalClassFormatException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

public class SeriesObject<T> extends DestroyableObject {
    private Class<T> typeParameterClass;
    private Class baseTypeClass;
    private final Method freeMethod;
    private final Method allocateMethod;
    private Method sTypeMethod = null;
    private Method clearMethod = null;

    private HashMap<T, Boolean> existingObjectInstances = new HashMap<>();
    private int releasedObjectInstances = 0;
    public SeriesObject(Class<T> typeParameterClass) throws IllegalClassFormatException {
        this.typeParameterClass = typeParameterClass;
        Optional<Method> optAllocateMethod = Arrays.stream(typeParameterClass.getMethods())
                .filter(method -> method.getName().equals("calloc"))
                .filter(method -> method.getParameters().length == 0)
                .filter(method -> method.getReturnType() == typeParameterClass)
                .findFirst();

        Optional<Method> freeMethod = Arrays.stream(typeParameterClass.getMethods())
                .filter(method -> method.getName().equals("free"))
                .filter(method -> method.getParameters().length == 0).findFirst();
        Optional<Method> clearMethod = Arrays.stream(typeParameterClass.getMethods())
                .filter(method -> method.getName().equals("clear"))
                .filter(method -> method.getParameters().length == 0).findFirst();
        Optional<Method> sTypeMethod = Arrays.stream(typeParameterClass.getMethods()).filter(method -> method.getName().equals("sType$Default")).findFirst();
        if (optAllocateMethod.isEmpty() || freeMethod.isEmpty()) {
            throw new IllegalClassFormatException("It is not lwjgl allocatable object");
        }
        if(sTypeMethod.isPresent())
            this.sTypeMethod = sTypeMethod.get();
        if(clearMethod.isPresent())
            this.clearMethod = clearMethod.get();
        this.allocateMethod = optAllocateMethod.get();
        this.freeMethod = freeMethod.get();
    }

    public T acquireObject(){
        try{
            for (Map.Entry<T, Boolean> entry : existingObjectInstances.entrySet()) {
                if(!entry.getValue()){
                    if(clearMethod!=null){
                        clearMethod.invoke(entry.getKey());
                    }
                    if(sTypeMethod!=null){
                        sTypeMethod.invoke(entry.getKey());
                    }
                    releasedObjectInstances--;
                    T res = entry.getKey();
                    existingObjectInstances.put(res, true);
                    return res;
                }
            }
            T res = (T) allocateMethod.invoke(null);
            existingObjectInstances.put(res, true);
            return res;
        }catch (Exception e){
            e.printStackTrace();
            return null;
        }
    }

    public void releaseObjectInstance(T instance){
        existingObjectInstances.put(instance, false);
        releasedObjectInstances++;
        if(releasedObjectInstances>3){
            finalizeObjectInstances();
        }
    }
    private void finalizeObjectInstances(){
        try{
            List<T> erasedObjects = new ArrayList<>();
            existingObjectInstances.forEach((key, val)->{
                if(!val){
                    erasedObjects.add(key);
                }
            });
            erasedObjects.forEach(element->{
                try {
                    freeMethod.invoke(element);
                    existingObjectInstances.remove(element);
                } catch (IllegalAccessException | InvocationTargetException e) {
                    throw new RuntimeException(e);
                }
            });
            releasedObjectInstances = 0;
        }catch (Exception e){
            e.printStackTrace();
        }

    }

    
    @Override
    public void destroy() {
        existingObjectInstances.forEach((element, val)->{
            try {
                freeMethod.invoke(element);
            } catch (Exception e){
                e.printStackTrace();
            }
        });
        destroyed = true;
    }


}
