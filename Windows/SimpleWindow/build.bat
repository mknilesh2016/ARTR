cls
del Window.exe
del Window.obj
del Window.res
cl.exe /c /EHsc Window.c
rc.exe Window.rc
link.exe Window.obj Window.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
del Window.obj
del Window.res
