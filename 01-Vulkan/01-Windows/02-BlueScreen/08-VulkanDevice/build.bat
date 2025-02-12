cls
if exist vk.exe (
    del vk.exe
)
cl.exe /c /EHsc /I "C:\VulkanSDK\Include" /W4 vk.c
rc.exe vk.rc
link.exe /LIBPATH:"C:\VulkanSDK\Lib" vk.obj vk.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
del vk.obj
del vk.res
