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

#include "vk.h"

// Vulkan related header files
// Instruct vulkan to include win32 platform specific code from vulkan.h
#define VK_USE_PLATFORM_WIN32_KHR
// Headers
#include <vulkan/vulkan.h>

// Vulkan related libraries
#pragma comment(lib,"vulkan-1.lib")

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
const char *gpSzAppName = "ARTR";

// Vulkan related global variables
// - Instance extensions related variables
uint32_t enabledInstanceExtensionCount = 0;
// VK_KHR_SURFACE_EXTENSION_NAME
// VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[2];
// Vulkan instance
VkInstance vkInstance = VK_NULL_HANDLE;

// Entry point function
int wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpszCmdLine,
    _In_ int iCmdShow
)
{
    // Function declarations
    VkResult Initialize(void);
    void Display(void);
    void Update(void);
    void Uninitialize(void);

    // Variable declarations
    WNDCLASSEX wndClassEx;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[255];
    BOOL bDone = FALSE;
    int iRet = 0;
    VkResult vkResult = VK_SUCCESS;

    // Code
    gpFile = fopen("app_log.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Failed to create log file. Exiting now..."), TEXT("File I/O Error"), MB_OK);
        SafeExit(-1);
    }
    else
    {
        LOGF("Log file created successfully.");
    }

    wsprintf(szAppName, TEXT("%s"), gpSzAppName);

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
        WS_EX_APPWINDOW,
        szAppName,
        TEXT("Nilesh Mahajan: Vulkan"),
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
        LOGF("WinMain: Unable to create window. Last error = %u.", GetLastError());
        SafeExit(-1);
    }

    ghwnd = hwnd;

    // Initialize
    vkResult = Initialize();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("WinMain: Initialize() failed with %i.", vkResult);
        SafeExit(-1);
    }
    else
    {
        LOGF("WinMain: Initialize() succeded.");
    }

    // Show window
    if (ShowWindow(hwnd, iCmdShow) == FALSE)
    {
        LOGF("WinMain: ShowWindow failed. Last error = %u.", GetLastError());
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
    case WM_CREATE:
        // memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
        // wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;

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

    case WM_KEYDOWN:
        switch (wParam)
        {
        case ESC_KEY_CODE:
            (void)DestroyWindow(hwnd);
            break;

        default:
            break;
        }
        break;

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

    case WM_SIZE:
        Resize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_CLOSE:
        Uninitialize();
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
    static WINDOWPLACEMENT wpPrev;
    static DWORD dwPrevStyle = 0;
    static MONITORINFO monitorInfo;

    wpPrev.length = sizeof(WINDOWPLACEMENT);
    
    // Variable declarations
    if (gbFullScreen == FALSE)
    {
        dwPrevStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwPrevStyle & WS_OVERLAPPEDWINDOW)
        {
            monitorInfo.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghwnd, &wpPrev) == TRUE &&
                GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &monitorInfo) == TRUE)
            {
                (void)SetWindowLong(ghwnd, GWL_STYLE, (dwPrevStyle & ~WS_OVERLAPPEDWINDOW));
                (void) SetWindowPos(
                    ghwnd,
                    HWND_TOP,
                    monitorInfo.rcMonitor.left,
                    monitorInfo.rcMonitor.top,
                    (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left),
                    (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top),
                    SWP_NOZORDER | SWP_FRAMECHANGED);
            }

            (void) ShowCursor(FALSE);
            gbFullScreen = TRUE;
        }
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, (dwPrevStyle | WS_OVERLAPPEDWINDOW));
        (void) SetWindowPlacement(ghwnd, &wpPrev);
        (void) SetWindowPos(
            ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        (void) ShowCursor(TRUE);
        gbFullScreen = FALSE;
    }
}

VkResult Initialize(void)
{
    // Function declarations
    VkResult CreateVulkanInstance(void);

    // Variable declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    vkResult = CreateVulkanInstance();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateVulkanInstance() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateVulkanInstance() succeded.");
    }

    return (vkResult);
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

    // Destroy vkInstance if valid
    if (vkInstance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = NULL;
        LOGF("Uninitialize: vkDestroyInstance succeded");
    }

    if (ghwnd != NULL)
    {
        (void) DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    if (gpFile != NULL)
    {
        LOGF("Uninitialize: Log file successfully closed.");
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

// ----------- Definitions of vulkan related functions --------------- //

VkResult CreateVulkanInstance(void)
{
    // Function declarations
    VkResult FillInstanceExtensionNames(void);

    // Variable declarations
    VkResult vkResult = VK_SUCCESS;
    VkApplicationInfo vkApplicationInfo;
    VkInstanceCreateInfo vkInstanceCreateInfo;

    // Code

    // Step-1: Fill and initialize required extension names and count global variables
    vkResult = FillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateVulkanInstance: FillInstanceExtensionNames() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateVulkanInstance: FillInstanceExtensionNames() succeded.");
    }

    // Step-2: Initialize struct VkApplicationInfo
    memset(&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpSzAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpSzAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

    // Step-3: Initialize struct VkInstanceCreateInfo
    memset(&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    // Step-4: Call vkCreateInstace() to get vkInstance in a global variable and do error checking
    vkResult = vkCreateInstance(
        &vkInstanceCreateInfo, // pCreateinfo
        NULL, // pAllocator of type const VkAllocationCallbacks*
        &vkInstance);

    if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        LOGF("CreateVulkanInstance: vkCreateInstance() failed wit VK_ERROR_INCOMPATIBLE_DRIVER(%d)", vkResult);
        return vkResult;
    }
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        LOGF("CreateVulkanInstance: vkCreateInstance() failed due to VK_ERROR_EXTENSION_NOT_PRESENT(%d)", vkResult);
        return vkResult;
    }
    else if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateVulkanInstance: vkCreateInstance() failed with due to unknown reason (%d)", vkResult);
        return vkResult;
    }
    else
    {
        LOGF("CreateVulkanInstance: vkCreateInstance() succeded");
    }

    return vkResult;
}

VkResult FillInstanceExtensionNames(void)
{
    // Variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t instanceExtensionCount = 0;
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    char** instanceExtensionNames_array = NULL;
    VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;

    // Step-1: Find how many instance extenstions are supported by the vulkan driver of
    // this version and keep in local variable
    vkResult = vkEnumerateInstanceExtensionProperties(
        NULL,                       // pLayerName is NULL for global extensions
        &instanceExtensionCount,    // pPropertyCount
        NULL);                      // pProperties
    if (vkResult != VK_SUCCESS)
    {
        LOGF("FillInstanceExtensionNames: First call to vkEnumerateInstanceExtensionProperties() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("FillInstanceExtensionNames: First call to vkEnumerateInstanceExtensionProperties() succeded.");
    }

    // Step-2: Allocate and fill struct VkExtensionProperties array
    vkExtensionProperties_array = (VkExtensionProperties*) malloc(instanceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        LOGF("FillInstanceExtensionNames: Unable to allocate VkExtensionProperties array");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    vkResult = vkEnumerateInstanceExtensionProperties(
        NULL,                         // pLayerName is NULL for global extensions
        &instanceExtensionCount,      // pPropertyCount
        vkExtensionProperties_array); // pProperties
    if (vkResult != VK_SUCCESS)
    {
        LOGF("FillInstanceExtensionNames: Second call to vkEnumerateInstanceExtensionProperties() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("FillInstanceExtensionNames: Second call to vkEnumerateInstanceExtensionProperties() succeded.");
    }

    // Step-3: Fill and display a local string array of extension names obtained from vkExtensionProperties_array
    instanceExtensionNames_array = (char **)malloc(instanceExtensionCount * sizeof(char*));
    if (instanceExtensionNames_array == NULL)
    {
        LOGF("FillInstanceExtensionNames: Unable to allocate instanceExtensionNames_array");
        free(vkExtensionProperties_array);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        int extensionNameLength = strlen(vkExtensionProperties_array[i].extensionName) + 1;
        instanceExtensionNames_array[i] = (char *)malloc(extensionNameLength * sizeof(char));
        if (instanceExtensionNames_array[i] != NULL)
        {
            strcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName);
            instanceExtensionNames_array[i][extensionNameLength] = 0;
        }
    }

    // Step-4 free vkExtensionProperties_array
    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    // Step-5: Find whether aboce extension names contain our required two surface extensions
    // and accordingly set 
    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        // Break out of loop if all surface extensions found
        if (vulkanSurfaceExtensionFound == VK_TRUE &&
            vulkanWin32SurfaceExtensionFound == VK_TRUE)
        {
            break;
        }

        if (vulkanSurfaceExtensionFound == VK_FALSE && 
            strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        else if (vulkanWin32SurfaceExtensionFound == VK_FALSE &&
            strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanWin32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
    }

    // Step-6: Free allocated instanceExtensionNames_array
    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        if (instanceExtensionNames_array[i] != NULL)
        {
            free(instanceExtensionNames_array[i]);
            instanceExtensionNames_array[i] = NULL;
        }
    }
    free(instanceExtensionNames_array);
    instanceExtensionNames_array = NULL;

    // Step-7: Display whether our vulkan driver supports our required extension names or not
    if (vulkanSurfaceExtensionFound == VK_FALSE)
    {
        LOGF("FillInstanceExtensionNames: Unable to locate VK_KHR_SURFACE_EXTENSION_NAME");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        LOGF("FillInstanceExtensionNames: Found VK_KHR_SURFACE_EXTENSION_NAME");
        vkResult = VK_SUCCESS;
    }
    if (vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        LOGF("FillInstanceExtensionNames: Unable to locate VK_KHR_WIN32_SURFACE_EXTENSION_NAME");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        LOGF("FillInstanceExtensionNames: Found VK_KHR_WIN32_SURFACE_EXTENSION_NAME");
        vkResult = VK_SUCCESS;
    }

    // Step-8: Print only supported instance names
    if (vkResult == VK_SUCCESS)
    {
        for (uint32_t i = 0; i < enabledInstanceExtensionCount; ++i)
        {
            LOGF("FillInstanceExtensionNames: Enabled vulkan instance extension name = %s", enabledInstanceExtensionNames_array[i]);
        }
    }

    return vkResult;
}