package com.kgaft.VulkanLib.Utils;

public abstract class DestroyableObject {
    protected boolean destroyed = false;

    public synchronized void destroy(){
        this.destroyed = true;
    }


    @Override
    protected synchronized void finalize() throws Throwable {

        if(!destroyed){
            destroy();
        }

        super.finalize();
    }



}