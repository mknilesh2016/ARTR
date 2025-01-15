Original Template from Pooja Waghmare
1. Window width and height 800 x 600
2. WinMain()
	- Function prototype declarations for Initialize(), Display(), Update() and Uninitialize()
	- Log file created using fopen()
	- Window created using CreateWindowEx()
	- Centered window using GetSystemMetrics()
	- Window background color black
	- Window with icon
	- Call to Initialize()
	- Calls to Display() and Update() in game loop
	- Call to Uninitialize() after game loop
3. WndProc()
	- WM_SETFOCUS & WM_KILLFOCUS
	- WM_ERASEBKGND : break from case for retained mode graphics and return 0 for immediate mode graphics
	- WM_CHAR : Case f/F for fullscreen
	- WM_KEYDOWN : for Escape to exit application
	- WM_SIZE : To call Resize()
	- WM_CLOSE : Exit application
	- WM_DESTROY : Exit application

4. ToggleFullScreen() using GetWindowLong(), SetWindowLong(), SetWindowPos() and GetMonitorInfo()
5. Initialize() : return 0
6. Resize() : Set height to 1 if it is 0
7. Display() : Empty
8. Update() : Empty
9. Uninitialize() : Exit fullscreen, destroy window and close file.
10. Window.h and Window.rc files for icon

Additions:
- Updated Icon and window title and other texts including log file names
- Added #pragma warning(disable : 4996) to disable fopen warning
- Added annotation prototypes for WinMain to get rid of error
- Added WS_EX_TOPMOST to extended styles in CreateWindowEx()
- Called ShowWindow() before Initialize()
- Added Error Handling
- Added SafeExit and LOGF() methods for consistency