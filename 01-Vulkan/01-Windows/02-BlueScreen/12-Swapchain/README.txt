// Swapchain
- 



Step-6: Swapchain is capable of storing transformed image before presentation which is called as 'pre-transform'.
While creating swapchain, we can decide whether to pre-transform or not the swapchain images (Pre-transform
also includes flipping of images)

Step-7: Get presentation mode for swapchain images using step-11

Step-8: According to above data declare, memset and initialize VkSwapchainCreateInfo structure

Step-9: At the end call             to create the swapchain.

Step-10: When done, destroy it in Uninitialize() by using vkDestroySwapchain vulkan API

