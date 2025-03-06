// Swapchain Images And Image Views

Steps:
1) Get swapchain image count in a global variable using vkGetSwapchainImagesKHR() API
2) Declare a global VkImageType array and allocate it to the swapchain image count using malloc()
3) Now call the same function again and fill this array.
4) Declare another global array of type VkImageView and allocate it to the sizeof swapchainImageCount
5) Declare and initialize VkImageViewCreateInfo struct except its ".image" member
6) Start a loop for swapchain-image-count and inside this loop, initialize above ".image" member to the SWapchainArrayuIndex that we've obtained and then call vkCreateImageView() API to fill above imageView array.
7) In Uninitialize() destroy ImageViews fron ImageViewsArray using VkDestroyImageViews()
8) In uninitialize(), now actually free the imageViewwArray using free
9) No need to destroy imageArray affter imageviewarray has been destroyed.
10) In Uninitialize() now actually free the swapchainImageArry using free().


