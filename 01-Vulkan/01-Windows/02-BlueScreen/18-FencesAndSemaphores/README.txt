// Fences and Semaphores

Steps 
1) Globally declare an array of fences of type VkFence (dynamic pointer type array). Additionally declare two semaphore objects of type VkSemaphore
2) In CreateSemaphore() User Defined Function (UDF) declare, memset and initialize VkSemaphoreCreateInfo structure.
3) Now call VkCreateSemaphore two times to create our two semaphore objects. Remember, both will use same VkSemaphoreCreateInfo struct.
4) In CreateFences() UDF, declare, memset and initialize VkFenceCreateInfo structure.
5) In this UDF, allocate our global fences array to the size of swapchain image count using malloc()
6) Now, in a loop, call vkCreateFence() to initialize our global fences array.
7) In UnInitialize(), first in a loop, with swapchainImageCount as a counter, destroy fences array objects using vkDestroyFence() and then actually free the allocated
fences array by using free().
8) Destroy both global semaphore objects with two separate objects to vkDestroySemaphore()


Vulkan Synchronisation:
" If you understand synchronisation in vulkan, you can understand vulkan. "

Types of supported objects:
1) vkDeviceWaitIdle: Host wait operation. Use OS devfined wait operation.
2) Semaphores: Wait for inter-queue operations on device. Its a hardware object.
3) Fences: Used to waiting host to complete something on device. To create fences, we need to take help of Operating System.
4) Barriers
5) Events
6) Locks

