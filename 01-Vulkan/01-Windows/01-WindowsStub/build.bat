cls
if exist vk.exe (
    del vk.exe
)
cl.exe /c /EHsc vk.c
rc.exe vk.rc
link.exe vk.obj vk.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
del vk.obj
del vk.res
