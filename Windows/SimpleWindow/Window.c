// ************************************************************************* //
//
// Name		:   Nilesh Mahajan
// Roll No.	:   ARTR01-109
// Program	:   SimpleWindow
//              Based on IntegratedWindow Template by Pooja Waghmare
// 
// ************************************************************************* //

// Disable fopen related warning
#pragma warning(disable : 4996)

// Header files
#include <windows.h>
#include <stdio.h>

#include "Window.h"

// Macro definitions
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
#define ESC_KEY_CODE    27

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LOGF(const char* format, ...);
void SafeExit(int iExitCode);

// Global variables
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;
FILE* gpFile = NULL;

// Entry point function
int wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpszCmdLine,
    _In_ int iCmdShow
)
{
    // Function declarations
    int Initialize(void);
    void Display(void);
    void Update(void);
    void Uninitialize(void);

    // Variable declarations
    WNDCLASSEX wndClassEx;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("Simple Window");
    BOOL bDone = FALSE;
    int iRet = 0;

    // Code
    gpFile = fopen("app_log.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Failed to create log file. Exiting now..."), TEXT("File I/O Error"), MB_OK);
        SafeExit(-1);
    }

    LOGF("Log file created successfully.");

    // Initialize WNDCLASSEX structure
    memset(&wndClassEx, 0, sizeof(WNDCLASSEX));
    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndClassEx.cbClsExtra = 0;
    wndClassEx.cbWndExtra = 0;
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = hInstance;
    wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClassEx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClassEx.lpszClassName = szAppName;
    wndClassEx.lpszMenuName = NULL;
    wndClassEx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // Register above class
    (void) RegisterClassEx(&wndClassEx);

    // Create the window
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW | WS_EX_TOPMOST,
        szAppName,
        TEXT("Nilesh Mahajan: Window"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL)
    {
        LOGF("Unable to create window. Last error = %u.", GetLastError());
        SafeExit(-1);
    }

    ghwnd = hwnd;

    // Show window
    if (ShowWindow(hwnd, iCmdShow) == FALSE)
    {
        LOGF("ShowWindow failed. Last error = %u.", GetLastError());
        SafeExit(-1);
    }

    // Initialize
    iRet = Initialize();
    if (iRet != 0)
    {
        LOGF("Initialize() failed with %i.", iRet);
        SafeExit(-1);
    }

    // Bring window to foreground and set focus on it
    (void) SetForegroundWindow(hwnd);
    (void) SetFocus(hwnd);

    // Game loop
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                (void) TranslateMessage(&msg);
                (void) DispatchMessage(&msg);
            }
        }
        else
        {
            if (gbActiveWindow == TRUE)
            {
                // Render the scene
                Display();

                // Update the scene
                Update();
            }
        }
    }

    Uninitialize();

    return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // Function declarations
    void ToggleFullScreen(void);
    void Resize(int, int);

    // Code
    switch (iMsg)
    {
    case WM_SETFOCUS:
        gbActiveWindow = TRUE;
        break;

    case WM_KILLFOCUS:
        gbActiveWindow = FALSE;
        break;

    case WM_ERASEBKGND:
        // break for retained mode graphics
        // WM_PAINT will paint the background
        break;

        // return 0 for immediate mode graphics
        // we don't want this msg to go to DefWindowProc()
        //return 0;

    case WM_CHAR:
        switch (wParam)
        {
        case 'f':
        case 'F':
            ToggleFullScreen();
            break;

        default:
            break;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case ESC_KEY_CODE:
            (void) DestroyWindow(hwnd);
            break;

        default:
            break;
        }
        break;

    case WM_SIZE:
        Resize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_CLOSE:
        (void) DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
    // Variable declarations
    static DWORD dwStyle;
    static WINDOWPLACEMENT wp;
    static MONITORINFO mi;

    // Code
    wp.length = sizeof(WINDOWPLACEMENT);

    if (gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            mi.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghwnd, &wp) == TRUE &&
                GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi) == TRUE)
            {
                (void) SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
                (void) SetWindowPos(
                    ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    (mi.rcMonitor.right - mi.rcMonitor.left),
                    (mi.rcMonitor.bottom - mi.rcMonitor.top),
                    SWP_NOZORDER | SWP_FRAMECHANGED);
            }

            (void) ShowCursor(FALSE);
            gbFullScreen = TRUE;
        }
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        (void) SetWindowPlacement(ghwnd, &wp);
        (void) SetWindowPos(
            ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        (void) ShowCursor(TRUE);
        gbFullScreen = FALSE;
    }
}


int Initialize(void)
{
    // Function declarations

    // Variable declarations

    // Code

    return (0);
}


void Resize(int width, int height)
{
    // Code
    if (height == 0)
    {
        height = 1;
    }
}


void Display(void)
{
    // Code
}


void Update(void)
{
    // Code
}


void Uninitialize(void)
{
    // Function declarations
    void ToggleFullScreen(void);

    // Code
    if (gbFullScreen == TRUE)
    {
        ToggleFullScreen();
    }

    if (ghwnd != NULL)
    {
        (void) DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    if (gpFile != NULL)
    {
        LOGF("Log file successfully closed.");
        (void) fclose(gpFile);
        gpFile = NULL;
    }
}

void LOGF(const char* format, ...)
{
    if (gpFile == NULL)
    {
        return;
    }

    va_list args;

    // Initialize the va_list
    va_start(args, format);

    // Write to the file using vfprintf
    (void) vfprintf(gpFile, format, args);

    // Add newline
    (void) fprintf(gpFile, "\n");

    // Close the file
    (void) fflush(gpFile);

    // Clean up the va_list
    va_end(args);
}

void SafeExit(int iExitCode)
{
    Uninitialize();
    exit(iExitCode);
}
