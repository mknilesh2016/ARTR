In the shaders uptil now there were no uniforms. Hence .bindingCount and .pBindings members of VkDescriptorSetLayoutCreateInfo
were not used.

But in this application and henceforth, our shader is going to have at least one uniform, means at least one DescriptorSet and
hence above two members will be used by the new type VkDescriptorSetLayoutBinding

Before creating actual descriptor pool, vulkan expects DescriptPoolSize

    // There are two ways to update -
    // 1. Writing directly to the shader
    // 2. Copying from one shader to another shader