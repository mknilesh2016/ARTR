Shaders are same as Ortho. Not with Inverted-Y
- Rasterization state: VK_FRONT_FACE_COUNTER_CLOCKWISE
- glm follows column major matrix just like opengl BUT unlike openGL, its matrix-array is 2D array 4x4, not 1D array of 16 elements


VertexData vertexData_position_index;
const uint32_t triangle_position_indices[] = { 0, 1, 2 };

        // Bind with the index buffer
        VkDeviceSize vkDeviceSize_index_offset_array[1];
        vkCmdBindIndexBuffer(vkCommandBuffer_array[0], vertexData_position_index.vkBuffer, vkDeviceSize_index_offset_array[0], VK_INDEX_TYPE_UINT32);

                // Here, we should call vulkan drawing functions
        vkCmdDrawIndexed(vkCommandBuffer_array[i], 3, 1, 0, 0, 0);

        