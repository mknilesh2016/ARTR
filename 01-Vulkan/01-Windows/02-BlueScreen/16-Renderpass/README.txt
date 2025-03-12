// Renderpass

1) Declare and initialize VkAttachmentDescription structure array (Array depends upon number of attachments)
Although we have only 1 attachment (color attachment) in this example, we will consider it as an array
2) Declare and initialize VkAttachmentReference structure which will have information about the attachment described above.
3) Declare and initialize VkSubpassDescription structure and keep information about above VkAttachmentReference structure.
4) Declare and initialize VkRenderPassCreateInfo and refer VkAttachmentDescription and VkSubpassDescription into it.
Remember attachment information in the form of ImageViews which will be used by frame buffer later. We also need to specify
inter-dependency between subpasses if needed.
5) Now call VkCreateRenderpass() to create actual renderpass. Remember 
6) In Uninitialize() destroy the renderpass by using vkDestroyRenderPass




