// Command Buffer

1) Declare and initialize struct VkCommandBufferAllocateInfo
2) Declare a command buffer array globally and allocate it to the sizeof swapchain image count (The number of command buffers are conventionally equal to number of swapchain images)
3) In a loop which is equal to swapchain image count, allocate each command buffer in this array by using VkAllocateCommandBuffers() API. Remember, at the time of allocation, all buffers are going to be empty. Later we
will record graphics/compute commands in them.
4) In Uninitialize, free each command buffer by using VkFreeCommandBuffers() API in a loop of size swapchain image count.
5) Free the actual command buffer array

primary command buffers can be submitted
secondary command buffers can be called within primary buffer



