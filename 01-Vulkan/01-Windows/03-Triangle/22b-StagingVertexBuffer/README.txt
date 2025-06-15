Algo Steps:
1. Create a local vertex buffer just like previously created vertex buffer but with different usage and different shading mode.
Identify it as a staging buffer. This buffer will be visible to HOST and can be mapped to device memory.
2. Then create thge usual vertex buffer visible only to the device.
3. Create one special command buffer.
4. Build above command buffer by vulkan's buffer copy command and copy data in the staging buffer in the staging buffer in step 1
to destination buffer in step 2. While doing this, use our queue and the command buffer to submit the work of buffer copy.
5. After DONE, free/destroy the command buffer.
6. Destroy the local staging vertex buffer as it's job is done.