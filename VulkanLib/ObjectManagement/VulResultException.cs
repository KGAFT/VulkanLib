using Silk.NET.Vulkan;

namespace VulkanLib.ObjectManagement;

public class VulResultException : Exception
{
    public static void checkResult(Result result)
    {
        if (result != Result.Success)
        {
            throw new VulResultException(result);
        }
    }
    public static void checkResult(String message, Result result)
    {
        if (result != Result.Success)
        {
            throw new VulResultException(message, result);
        }
    }
    public VulResultException(Result result)
    {
        message = result.ToString();
    }

    public VulResultException(String message, Result result)
    {
        this.message = message+" "+result.ToString();
    }
    private String message;
    public override Exception GetBaseException()
    {
        return new Exception(message);
    }

    public override string ToString()
    {
        return message;
    }
}