cls
if exist vk.exe (
    del vk.exe
)

C:\VulkanSDK\Bin\glslangValidator.exe -V -H -o shader.vert.spv shader.vert
C:\VulkanSDK\Bin\glslangValidator.exe -V -H -o shader.frag.spv shader.frag

cl.exe /c /EHsc /I "C:\VulkanSDK\Include" vk.c
rc.exe vk.rc
link.exe /LIBPATH:"C:\VulkanSDK\Lib" vk.obj vk.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
del vk.obj
del vk.res
