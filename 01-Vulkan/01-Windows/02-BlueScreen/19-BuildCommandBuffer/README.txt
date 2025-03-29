// BuildCommandBuffer

- We need to reset the command buffer before building it.
- In order to build command buffer we need to -
Call BeginComnmandBuffer()
    BeginRenderPass()
        Do Actual Renders
    EndRenderPass()
Call EndComnmandBuffer()

Steps:
1) Start a loop with swapchainImageCount as the counter
2) Inside the loop, call vkResetCommandBuffer() to reset the contents of the command buffer
3)      Declare, memset, initialize VkCommandBufferBeginInfo struct
4)      Call vkBeginCommandBuffer() to record vulkan drawing related commands. Do error checking.
5)      Declare, memset and initialize struct array of VkClearValue type. 
            Remember, internally this is a union. 
            This number depends upon number of attachments to framebuffer. As we have only one attachment i.e. color attachment, our array is of size 1. (.color member is meaningful in color attachment)
            To this color member, we need to assign VkClearColorValue struct. To do this, declare globally VkClearColorValue struct variable and memset and initialize it in Initialize() function.
            Remember, we're going to clear .color member of VkClearValue structure by VkClearColorValue structure because in prog 16-RenderPass, we specified .loadOp member of vkAttachmentDescription struct to VK_ATTACHMENT_LOAD_OP_CLEAR
6) Then declare, memset and initialize VkRenderPassBeginInfo structure
7) Then being render pass by VkCmdBeginRenderPass(). Remember, the code written inside BeginRenderPass() and EndRenderPass() itself is the code of subpass, if no subpass is explicitely created. 
    IOW there is at least 1 subpass for a render pass (like main thread in process)
8) End the render pass by calling VkCmdEndRenderPass()
9) End the recording of command buffer by calling VkEndCommandBuffer()
10) Close the loop