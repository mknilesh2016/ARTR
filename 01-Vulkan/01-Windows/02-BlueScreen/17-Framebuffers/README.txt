// Framebuffers

Framebuffer is a set of images that is going to be rendered. That is why framebuffer always has a reference to a renderpass.
One framebuffer can be common to multiple render passes. The restriction is that those renderpasses should be compatible.
Compatible means attachments must be same.

Destroying framebuffer does not destroy images attached to it. Hence you can attach these images later.
Framebuffers are furhter used by command buffers. Hence before destroying them, ensure that framebuffer has completed their execution.

The number of framebuffers should be similar to number of swapchain images.

Steps:
1) Declare an array of VkImageView equal to the number of color attachments means in our example would be array of 1 member
2) Declare and initialize VkFrameBufferCreateAndInitialize
3) Start a loop for swapchainImageCount and call vkCreateFramebuffer() API to create framebuffers
4) In uninitialize destroy the framebuffers in a loop for swapchainImageCount