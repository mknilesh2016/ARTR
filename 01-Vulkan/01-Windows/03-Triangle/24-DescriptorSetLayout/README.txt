Steps:
1) Globally declare object of type VkDescriptorSetLayout and initialize to VK_NULL_HANDLE
2) In Initialize(), declare and call UDF CreateDescriptorSetLayout() maintaining the convention
of declaring and calling it after CreateShaders() and before CreateRenderPass()
3) While writing this UDF, declare memset and initialize struct VkDescriptorSetLayoutCreateInfo particularly
its two members; a) bindingCount b) pBindings array. Then call vkCreateDescriptorSetLayout() vulkan api with
address of above initialized structure and get the required global vkDescriptorSetLayout in its last
parameter.
5) In Uninitialize() call vkDestroyDescriptorSetLayout() vulkan APIa.



Resources:

Descriptor set 1: ---------------
- texture                       |
- sampler                       | DescriptorSet Layout _____________  Ordered ^^
- sampler                       |                                   |
- buffer          ______________|                                   |
                                                                    |
Descriptor set 2:_______________|                                   | Pipeline Layout
- texture                       |                                   |
- texture                       |                                   |
- texture                       | DescriptorSet layout _____________|
- sampler                       |
- sampler                       |
- sampler                       |
- buffer_________________________


    vkDescriptorSetLayoutCreateInfo.pBindings = NULL;

    // pBindings is array of VkDescriptorSetLayoutBinding which has 5 members
    // uint32_t binding => relates to binding_point parameter to glBindBuffer. the point was fixed in GL but its flexible in vulkan
    //         integer value where you want to bind descriptor set
    // VkDescriptorSetType descriptorType => type of resource descriptors are used for this binding.
            // typedef enum VkDescriptorType {
            //     VK_DESCRIPTOR_TYPE_SAMPLER = 0,
            //     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
            //     VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
            //     VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
            //     VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
            //     VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
            //     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
            //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
            //     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
            //     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
            //     VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
            //   // Provided by VK_VERSION_1_3
            //     VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK = 1000138000,
            //   // Provided by VK_KHR_acceleration_structure
            //     VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000,
            //   // Provided by VK_NV_ray_tracing
            //     VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
            //   // Provided by VK_QCOM_image_processing
            //     VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM = 1000440000,
            //   // Provided by VK_QCOM_image_processing
            //     VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM = 1000440001,
            //   // Provided by VK_EXT_mutable_descriptor_type
            //     VK_DESCRIPTOR_TYPE_MUTABLE_EXT = 1000351000,
            //   // Provided by VK_NV_partitioned_acceleration_structure
            //     VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV = 1000570000,
            //   // Provided by VK_EXT_inline_uniform_block
            //     VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
            //   // Provided by VK_VALVE_mutable_descriptor_type
            //     VK_DESCRIPTOR_TYPE_MUTABLE_VALVE = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
            // } VkDescriptorType;
    // uint32_t descriptorCount;
    // VkShaderStageFlags stageFlags: =>  is a bitmask of VkShaderStageFlagBits specifying which pipeline shader stages can access a resource for this binding. VK_SHADER_STAGE_ALL is a shorthand specifying that all defined shader stages, including any additional stages defined by extensions, can access the resource.
    //          If a shader stage is not included in stageFlags, then a resource must not be accessed from that stage
    //          via this binding within any pipeline using the set layout. Other than input attachments which are limited
    //          to the fragment shader, there are no limitations on what combinations of stages can use a descriptor binding,
    //          and in particular a binding can be used by both graphics stages and the compute stage.
    // const VkSampler* pImmutableSamplers => Used when using immutable samplers