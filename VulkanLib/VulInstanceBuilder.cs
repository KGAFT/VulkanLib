namespace VulkanLib;

public class VulInstanceBuilder
{
    private List<String> layers = new();
    private List<String> extensions = new(){"VK_KHR_get_physical_device_properties2", "VK_EXT_swapchain_colorspace"};
    private bool debugLogging = false;
    private String applicationName = "VulkanLibApp";
    
    public void addLayer(String layerName) => layers.Add(layerName);
    public void addExtension(String extensionName) => extensions.Add(extensionName);

    public void presetForDebugging()
    {
        layers.Add("VK_LAYER_KHRONOS_validation");
        extensions.Add("VK_EXT_debug_utils");
    }
    
    public void setAplicationName(string aplicationName) => applicationName = aplicationName;
    
    public List<String> getLayers() => layers;
    public List<String> getExtensions() => extensions;
    
    public bool getDebugLogging() => debugLogging;
    public String getApplicationName() => applicationName;
    
}