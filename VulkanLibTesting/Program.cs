// See https://aka.ms/new-console-template for more information

using VulkanLib;
VulInstanceBuilder builder = new VulInstanceBuilder();
builder.presetForDebugging();
VulInstance instance = new(builder);
