// Create VulkanDevice

Steps:
1. Declare CreateVulkanDevice user defined function.
2. Call previously created fillDeviceExtensionNames() in it.
3. Declare and initialize VkDeviceCreateInfo structure. Use previously obtained enabledDeviceExtensionCount 
and enabledDeviceExtensionArray to initialize this structure.
4. Now call vkCreateDevice() vulkan API to actually create the vulkan device and do error checking.
5. Destroy this device when done. 
<<-- Before destroying the device, ensure that all operations on that device are finished -->>
<<-- Till then wait on that device -->>

