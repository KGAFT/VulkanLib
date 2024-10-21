using Silk.NET.Vulkan;
using VulkanLib.ObjectManagement;

namespace VulkanLib.Device.LogicalDevice;

public class VulLogicalQueue : DestroyableObject
{
    public unsafe VulLogicalQueue(Queue baseQueue, Silk.NET.Vulkan.Device device, bool supportPresentation, QueueFlags queueType, uint index)
    {
        this.queue = baseQueue;
        this.device = device;
        this.supportPresentation = supportPresentation;
        this.queueType = queueType;
        this.index = index; 
        CommandPoolCreateInfo poolInfo = new();
        poolInfo.SType = StructureType.CommandPoolCreateInfo;
        poolInfo.QueueFamilyIndex = index;
        poolInfo.Flags =
            CommandPoolCreateFlags.TransientBit | CommandPoolCreateFlags.ResetCommandBufferBit;
        VulResultException.checkResult("Failed to allocate queue command pool", Vk.GetApi().CreateCommandPool(device, &poolInfo, null, out commandPool));
    }
    private Queue queue;
    private bool supportPresentation;
    private QueueFlags queueType;
    private CommandPool commandPool = new();
    private Silk.NET.Vulkan.Device device;
    private uint index;
    private CommandBufferAllocateInfo allocInfo = new();
    private CommandBufferBeginInfo beginInfo = new();
    private SubmitInfo submitInfo = new();

    public CommandBuffer beginSingleTimeCommands()
    {
        allocInfo.SType = StructureType.CommandBufferBeginInfo;
        allocInfo.Level = CommandBufferLevel.Primary;
        allocInfo.CommandPool = commandPool;
        allocInfo.CommandBufferCount = 1;
        CommandBuffer res = new();
        VulResultException.checkResult("Failed to allocate single time commands: ", Vk.GetApi().AllocateCommandBuffers(device, allocInfo, out res));
        
        beginInfo.SType = StructureType.CommandBufferBeginInfo;
        beginInfo.Flags = CommandBufferUsageFlags.OneTimeSubmitBit;
        VulResultException.checkResult("Failed to begin single time commands: ",  Vk.GetApi().BeginCommandBuffer(res, beginInfo));
        return res;
    }

    public unsafe void endSingleTimeCommands(CommandBuffer cmd)
    {
        VulResultException.checkResult("Failed to end command buffer: " ,Vk.GetApi().EndCommandBuffer(cmd));
        submitInfo.SType = StructureType.SubmitInfo;
        submitInfo.CommandBufferCount = 1;
        submitInfo.PCommandBuffers = &cmd;
        ReadOnlySpan<SubmitInfo> subm = new(submitInfo);
        VulResultException.checkResult("Failed to submit queue", Vk.GetApi().QueueSubmit(queue, subm, new Fence(null)));
        Vk.GetApi().QueueWaitIdle(queue);
        Vk.GetApi().FreeCommandBuffers(device, commandPool, 1, &cmd);
    }
    public Queue getQueue() => queue;
    public CommandPool getCommandPool() => commandPool;
    public uint getIndex() => index;
    public bool isSupportPresentation() => supportPresentation;
    
    public QueueFlags getQueueType() => queueType;
    
    public override unsafe void destroy()
    {
        destroyed = true;
        Vk.GetApi().DestroyCommandPool(device, commandPool, null);
    }
}