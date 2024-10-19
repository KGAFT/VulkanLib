namespace VulkanLib.ObjectManagement;

public abstract class DestroyableObject
{
    protected bool destroyed = false;

    public virtual void destroy()
    {
        this.destroyed = true;
    }

    ~DestroyableObject()
    {
        if (!destroyed)
        {
            destroy();
        }
    }
}