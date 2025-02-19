// SurfaceFormatAndSurfaceColorSpace


- B8G8R8A8_UNORM is the minimum denominator color format in vulkan
- Color Space -> Depending upon where to present (display-RGB/printing-CMYK)

Types of RGB
- Linear
- Non Linear 

Steps:
1. Call vkGetPhysicalDeviceSurfaceFormatsKHR function to retrieve count of supported color formats followed
by the color formats.
2. Declare and allocate array of VkSurfaceFormatKHR curresponding to above call. The structure has two
members vkFormat and vkColorSpaceKHR
3. According to the contents of the above filled array decide surface color format and color space.