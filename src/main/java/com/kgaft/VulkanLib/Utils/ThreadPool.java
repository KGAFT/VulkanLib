package com.kgaft.VulkanLib.Utils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ThreadPool {
    private List<Runnable> tasks = new ArrayList<>();
    private Thread superVisorThread = null;
    private int threadCount;
    public ThreadPool(int threadCount){
        this.threadCount = threadCount;
    }
    public void addTask(Runnable task){
        if(superVisorThread==null || !superVisorThread.isAlive()){
            tasks.add(task);
        } else {
            throw new RuntimeException("Failed to add task, you cannot add task to this thread pool, until its not finished work");
        }

    }
    public void executeTasks(){
        superVisorThread = new Thread(superVisorMain);
        superVisorThread.run();
    }

    public void waitForFinish() throws InterruptedException {
        superVisorThread.join();
    }
    private Runnable superVisorMain = () -> {
        int freeThreadCount = threadCount;
        HashMap<Thread, Runnable> usedThreads = new HashMap<>();
        while(!tasks.isEmpty()){
            if(freeThreadCount>0){
                Runnable task = tasks.get(0);
                Thread thread = new Thread(task);
                usedThreads.put(thread, task);

                thread.run();
                freeThreadCount--;
            }
            if(freeThreadCount<=0){
                List<Thread> removeThreads = new ArrayList<>();
                for (Map.Entry<Thread, Runnable> entry : usedThreads.entrySet()) {
                    Thread thread = entry.getKey();
                    Runnable task = entry.getValue();
                    if (!thread.isAlive()) {
                        tasks.remove(task);
                        freeThreadCount++;
                        removeThreads.add(thread);
                    }
                }
                removeThreads.forEach(usedThreads::remove);
            }
        }
    };
}
