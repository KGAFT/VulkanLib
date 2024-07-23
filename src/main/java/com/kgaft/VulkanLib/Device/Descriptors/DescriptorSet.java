package com.kgaft.VulkanLib.Device.Descriptors;

import com.kgaft.VulkanLib.Device.LogicalDevice.LogicalDevice;
import com.kgaft.VulkanLib.Utils.LwjglObject;
import org.lwjgl.vulkan.VkCommandBuffer;
import org.lwjgl.vulkan.VkWriteDescriptorSet;

import java.lang.instrument.IllegalClassFormatException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import static org.lwjgl.vulkan.VK13.*;
public class DescriptorSet {
    protected long[] descriptorSets;
    protected LogicalDevice device;
    private List<DescriptorBufferInfo> buffersInfo = new ArrayList<>();
    private List<DescriptorImageInfo> imagesInfo = new ArrayList<>();
    private List<DescriptorAccelerationStructureInfo> asInfo = new ArrayList<>();
    private int imageInfoPerInstanceAmount = 0;
    private int bufferInfoPerInstanceAmount = 0;

    private int imgCount = 0;
    private int buffCount = 0;
    protected DescriptorSet(){}

    public void addBufferInfo(DescriptorBufferInfo bufferInfo) {
        buffersInfo.add(bufferInfo);
    }
    public void updateDescriptors() throws IllegalClassFormatException {
        LwjglObject<VkWriteDescriptorSet.Buffer> writes = packsDescriptorWrites();
        for (long item : descriptorSets) {

            while(writes.get().hasRemaining()){
                writes.get().get().dstSet(item);
            }
            writes.get().rewind();
            if (imageInfoPerInstanceAmount == 0 && bufferInfoPerInstanceAmount == 0) {
                vkUpdateDescriptorSets(device.getDevice(), writes.get(), null);
            } else {
                vkUpdateDescriptorSets(device.getDevice(), writes.get(), null);
                writes = packsDescriptorWrites();
            }
        }
    }

    public void addImageInfo(DescriptorImageInfo imageInfo) {
        imagesInfo.add(imageInfo);
    }

    public void addAccelerationStructureInfo(DescriptorAccelerationStructureInfo acsInfo) {
        asInfo.add(acsInfo);
    }

    public void clearObjectInfos(){
        buffersInfo.clear();
        imagesInfo.clear();
        asInfo.clear();
    }

    public void bindDescriptor(int bindPoint, int currentInstance, VkCommandBuffer cmd,
                                       long pipelineLayout) {
        long[] dsc = new long[]{descriptorSets[currentInstance]};
        vkCmdBindDescriptorSets(cmd, bindPoint, pipelineLayout, 0, dsc, null);
    }

    private LwjglObject<VkWriteDescriptorSet.Buffer> packsDescriptorWrites() throws IllegalClassFormatException {
        if(imageInfoPerInstanceAmount == 0 && bufferInfoPerInstanceAmount == 0){
            LwjglObject<VkWriteDescriptorSet.Buffer> descriptorWrites = new LwjglObject<>(VkWriteDescriptorSet.class, VkWriteDescriptorSet.Buffer.class, buffersInfo.size()+imagesInfo.size()+asInfo.size());
            AtomicInteger counter = new AtomicInteger();
            buffersInfo.forEach(element->{
                descriptorWrites.get().get(counter.get()).sType$Default();
                descriptorWrites.get().get(counter.get()).dstSet(descriptorSets[0]);
                descriptorWrites.get().get(counter.get()).dstBinding(element.getBinding());
                descriptorWrites.get().get(counter.get()).dstArrayElement(0);
                descriptorWrites.get().get(counter.get()).descriptorType(element.getDescriptorType());
                descriptorWrites.get().get(counter.get()).descriptorCount(element.getBase().get().capacity());
                descriptorWrites.get().get(counter.get()).pBufferInfo(element.getBase().get());
                counter.getAndIncrement();
            });
            imagesInfo.forEach(element->{
                descriptorWrites.get().get(counter.get()).sType$Default();
                descriptorWrites.get().get(counter.get()).dstSet(descriptorSets[0]);
                descriptorWrites.get().get(counter.get()).dstBinding(element.getBinding());
                descriptorWrites.get().get(counter.get()).dstArrayElement(0);
                descriptorWrites.get().get(counter.get()).descriptorType(element.getDescriptorType());
                descriptorWrites.get().get(counter.get()).descriptorCount(element.getBase().get().capacity());
                descriptorWrites.get().get(counter.get()).pImageInfo(element.getBase().get());
                counter.getAndIncrement();
            });
            asInfo.forEach(element->{
                descriptorWrites.get().get(counter.get()).sType$Default();
                descriptorWrites.get().get(counter.get()).dstSet(descriptorSets[0]);
                descriptorWrites.get().get(counter.get()).dstBinding(element.getBinding());
                descriptorWrites.get().get(counter.get()).dstArrayElement(0);
                descriptorWrites.get().get(counter.get()).descriptorType(element.getDescriptorType());
                descriptorWrites.get().get(counter.get()).descriptorCount(1);
                descriptorWrites.get().get(counter.get()).pNext(element.getBase().get());
                counter.getAndIncrement();
            });
            return descriptorWrites;
        } else {
            if(buffCount==bufferInfoPerInstanceAmount*descriptorSets.length){
                buffCount = 0;
            }
            if(imgCount==imageInfoPerInstanceAmount*descriptorSets.length){
                imgCount = 0;
            }
            LwjglObject<VkWriteDescriptorSet.Buffer> res = new LwjglObject<>(VkWriteDescriptorSet.class, VkWriteDescriptorSet.Buffer.class, imageInfoPerInstanceAmount+bufferInfoPerInstanceAmount);
            int c = 0;
            int startBuff = buffCount;
            for(; buffCount<startBuff+bufferInfoPerInstanceAmount; buffCount++){
                res.get().get(c).sType$Default();
                res.get().get(c).dstSet(descriptorSets[0]);
                res.get().get(c).dstBinding(buffersInfo.get(buffCount).getBinding());
                res.get().get(c).dstArrayElement(0);
                res.get().get(c).descriptorType(buffersInfo.get(buffCount).getDescriptorType());
                res.get().get(c).descriptorCount(buffersInfo.get(buffCount).getBase().get().capacity());
                res.get().get(c).pBufferInfo(buffersInfo.get(buffCount).getBase().get());
                c++;
            }
            int startImg = imgCount;
            for(; imgCount<startImg+bufferInfoPerInstanceAmount; imgCount++){
                res.get().get(c).sType$Default();
                res.get().get(c).dstSet(descriptorSets[0]);
                res.get().get(c).dstBinding(imagesInfo.get(imgCount).getBinding());
                res.get().get(c).dstArrayElement(0);
                res.get().get(c).descriptorType(imagesInfo.get(imgCount).getDescriptorType());
                res.get().get(c).descriptorCount(imagesInfo.get(imgCount).getBase().get().capacity());
                res.get().get(c).pImageInfo(imagesInfo.get(imgCount).getBase().get());
                c++;
            }
            return res;
        }

    }

    public void setImageInfoPerInstanceAmount(int imageInfoPerInstanceAmount) {
        this.imageInfoPerInstanceAmount = imageInfoPerInstanceAmount;
    }

    public void setBufferInfoPerInstanceAmount(int bufferInfoPerInstanceAmount) {
        this.bufferInfoPerInstanceAmount = bufferInfoPerInstanceAmount;
    }
}
