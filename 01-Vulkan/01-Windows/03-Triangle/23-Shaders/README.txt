Steps:
1) Write shaders and compile them to SPIR-V using shader compilation tools that we received in vulkan SDK.
2) Globally declare two shader module object variables of VkShaderModuleType to hold vulkan compatible vertex shader module object and fragment shader VkShaderModuleTypeobject respectively.
3) Declare prototype a UDF CreateShaders() in Initialize() following a convention - After CreateVertexBuffer() function and before CreateRenderPass()
4) Using same above convention, call CreateShaders() between the calls of aove two.
5) Maintaining th4 same above convention while defining CreateShaders() between the definition of the above two.
6) Inside our function -
    i) Open the sahder file, set the file pointer at the end of the file; find the byte size of the shader file data; reset the file pointer to the begining of the file;
    allocate a character buffer of file size and read shader file data into it; finally close the file. Do all these things using conventional file IO.
7) Declare and memset struct VkShaderModuleCreateInfo and specifiy above fileSize and buffer while initializing it.
8) Call vkCreateShaderModule() vulkan API; pass above struct's pointer to it as parameter to it and obtain shadermoduleObject in global variable that we decvlared in step 2.
9) Free the shader code buffer which we allocated in step 6.
10) Assuming we did above 4 steps (6-9) for vertex shader, repeat them all for fragmentshader too.
11) IN Uninitialize(), destroy both global shader objects using vkDestroyShaderModule() vulkan API.

Tools:
C:\VulkanSDK\Bin\glslangvalidator.exe <= Khronos official shader compiler
C:\VulkanSDK\Bin\glslc.exe
C:\VulkanSDK\Bin\glslang.exe

C:\VulkanSDK\Bin\spirv-dis.exe
        

"Vulkan accepts only SPIR-V format shaders"
SPIR-V "Standard for Portable Intermidiate Representation - Vulkan": Rendering technology portable so can be used in DX, Metal and across all GPU

It is recommended to give extension of .spv to compiled shaders
e.g. shader.virt.spv and shader.frag.spv


C:\VulkanSDK\Bin\glslangValidator.exe -V -H -o shader.vert.spv shader.vert
C:\VulkanSDK\Bin\glslangValidator.exe -V -H -o shader.frag.spv shader.frag

Powershell:
Format-Hex -Path .\shader.vert.spv | Select-Object -First 1
> Print the file in hexadecimal in powershell 

03 02 23 07: 0x07230203 => SPIR-V magic number

PS E:\ARTR\01-Vulkan\01-Windows\03-Triangle\23-Shaders> Format-Hex -Path .\shader.vert.spv | Select-Object -First 1


           Path: E:\ARTR\01-Vulkan\01-Windows\03-Triangle\23-Shaders\shader.vert.spv

           00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

00000000   03 02 23 07 00 00 01 00 0B 00 08 00 15 00 00 00  ..#.............

Linux:
xxd -e shader.vert.spv | head -n 1

SPIR-V writes its format in SSA i.e. Single Static Assignment for better optimization
<Each Virtual register tokens appear only once) 

glslangvalidator checks extension of the shader for compiler -
.vert
.frag
.tesc
.tese
.geom
.comp

THere is option -S to specify shader stage if you do not want to specify etension.
