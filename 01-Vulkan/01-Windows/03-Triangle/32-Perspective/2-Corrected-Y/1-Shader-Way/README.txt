change

    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // front facing triangle orientation to be used for culling

In vertex shader -

    // flip the Y- co-ordinate to correct y-axis
    gl_Position.y = -gl_Position.y;