Shaders are same as Ortho.

- While drawing triangle_list way is recommended since there is no triangle_fan way in METAL and D3D12.
- As vulkan on iOS and Mac runs on METAL (using molten VK), vulkan does not allow triangle_fan there.