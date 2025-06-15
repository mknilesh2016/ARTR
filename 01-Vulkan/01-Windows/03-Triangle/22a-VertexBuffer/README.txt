Steps:
1) Globally declare a structure holding vertex buffer related two things -
    a) VkBuffer object
    b) VkDeviceMemory object
    We will call it as struct VertexData and declare a global variable of this structure named vertexData_position.
2) Declare UDF createVertexBuffer(). Write its prototype below createCommandBuffer() and above createRenderPass()
and also call it between the calls of these two.
3) Implement createVertexBuffer() and inside declare our triangle's position array.
4) memset() our global vertexData_position
5) Declare and memset VkBufferCreateInfo struct. It has 8 members, we will use 5. Out of them 2 are very important
    a) usage
    b) size
6) Call vkCreateBuffer() api in the "vertexData_position.vkBuffer"
7) Declare and memset struct VkMemoryRequirement and then call vkGetBufferMemoryRequirement() api to get the memory requirements
8) To actually allocate the required memory we need to call vkAllocateMemory(). But before that we need to declare and memset
VkMemoryAllocateInfo struct. Important members of this struct are ".memoryTypeIndex" and ".allocationSize". 
    For allocationSize: use the size obtained from VkMemoryRequirement.
    For memoryTypeIndex: 
        a) Start the loop for vkPhysicalDeviceMemoryProperties.memoryTypeCount. Inside the loop, check VkMemoryRequirement.memoryTypeBits
        contains 1 or not. 
            If yes, check vkPhysicalDeviceMemoryProperties.memoryType[i].propertyFlags member contains VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
                If yes, then this i'th index will be our memoryTypeIndex. If found, break out of the loop.
                If no, continue the loop by right shifting VkMemoryRequirement.memoryType bits.
    Now call vkAllocateMemory to get the required vkMem
        




