// ************************************************************************* //
//
// Name		:   Nilesh Mahajan
// Roll No.	:   ARTR01-109
// Program	:   02-BlueScreen/07-DeviceExtensions
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
// Vulkan Presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;
// Vulkan physical device related global variables
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
// Added in this version
uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevices_array = NULL;
// - Device extensions related variables
uint32_t enabledDeviceExtensionCount = 0;
// VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char *enabledDeviceExtensionNames_array[1];

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
    VkResult GetSupportedSurface(void);
    VkResult GetPhysicalDevice(void);
    VkResult PrintVkInfo(void);
    VkResult FillDeviceExtensionNames(void);

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

    // Create Vulkan presentation surface
    vkResult = GetSupportedSurface();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: GetSupportedSurface() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: GetSupportedSurface() succeded.");
    }

    // Select required physical device and its queue family index
    vkResult = GetPhysicalDevice();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: GetPhysicalDevice() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: GetPhysicalDevice() succeded.");
    }

    // Print vulkan info
    vkResult = PrintVkInfo();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: PrintVkInfo() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: PrintVkInfo() succeded.");
    }

    // Fill and initialize required device extension names and count global variables
    vkResult = FillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: FillDeviceExtensionNames() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: FillDeviceExtensionNames() succeded.");
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

    // No need to destroy selected physical device.

    // Destroy vkSurfaceKHR if valid
    if (vkSurfaceKHR != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = NULL;
        LOGF("Uninitialize: vkDestroySurfaceKHR succeded");
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
    // Now passed in enabledExtensionNames along with enabledInstanceExtensionCount are enabled here

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
        LOGF("FillInstanceExtensionNames: Second call to vkEnumerateInstanceExtensionProperties() succeded with %d instance extensions", instanceExtensionCount);
    }

    // Step-3: Fill and display a local string array of extension names obtained from vkExtensionProperties_array
    instanceExtensionNames_array = (char **)malloc(instanceExtensionCount * sizeof(char*));
    if (instanceExtensionNames_array == NULL)
    {
        LOGF("FillInstanceExtensionNames: Unable to allocate instanceExtensionNames_array");
        free(vkExtensionProperties_array);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    LOGF("FillInstanceExtensionNames: Supported instance extensions");
    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        int extensionNameLength = strlen(vkExtensionProperties_array[i].extensionName) + 1;
        instanceExtensionNames_array[i] = (char *)malloc(extensionNameLength * sizeof(char));
        if (instanceExtensionNames_array[i] != NULL)
        {
            strcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName);
            instanceExtensionNames_array[i][extensionNameLength-1] = 0;
            LOGF("\t%s", instanceExtensionNames_array[i]);
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
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        LOGF("FillInstanceExtensionNames: Found VK_KHR_SURFACE_EXTENSION_NAME");
    }
    if (vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        LOGF("FillInstanceExtensionNames: Unable to locate VK_KHR_WIN32_SURFACE_EXTENSION_NAME");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        LOGF("FillInstanceExtensionNames: Found VK_KHR_WIN32_SURFACE_EXTENSION_NAME");
    }

    // Step-8: Print only supported instance names
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; ++i)
    {
        LOGF("FillInstanceExtensionNames: Enabled vulkan instance extension name = %s", enabledInstanceExtensionNames_array[i]);
    }

    return vkResult;
}

VkResult GetSupportedSurface(void)
{
    // Variable declarations
    VkResult vkResult = VK_SUCCESS;
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateinfoKHR;

    // Code
    memset(&vkWin32SurfaceCreateinfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
    vkWin32SurfaceCreateinfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateinfoKHR.pNext = NULL;
    vkWin32SurfaceCreateinfoKHR.flags = 0;
    vkWin32SurfaceCreateinfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateinfoKHR.hwnd = ghwnd;

    vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateinfoKHR, NULL, &vkSurfaceKHR);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetSupportedSurface: vkCreateWin32SurfaceKHR() failed with (%d)", vkResult);
        return vkResult;
    }
    else
    {
        LOGF("GetSupportedSurface: vkCreateWin32SurfaceKHR() succeded");
    }

    return vkResult;
}

VkResult GetPhysicalDevice(void)
{
    // Variable declarations
    VkResult vkResult = VK_SUCCESS;
    VkBool32 bFound = VK_FALSE;

    // Code
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetPhysicalDevice: First call to vkEnumeratePhysicalDevices() failed with %i.", vkResult);
        return (vkResult);
    }
    else if (physicalDeviceCount == 0)
    {
        LOGF("GetPhysicalDevice: vkEnumeratePhysicalDevices resulted into 0 physical devices");
        return (vkResult);
    }
    else
    {
        LOGF("GetPhysicalDevice: First call to vkEnumeratePhysicalDevices() succeded with %d devices.", physicalDeviceCount);
    }

    // Allocate and initialize devices array
    vkPhysicalDevices_array = (VkPhysicalDevice *)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if (vkPhysicalDevices_array == NULL)
    {
        LOGF("GetPhysicalDevice: Unable to allocate vkPhysicalDevices_array");
        free(vkPhysicalDevices_array);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevices_array);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetPhysicalDevice: Second call to vkEnumeratePhysicalDevices() failed with %i.", vkResult);
        free(vkPhysicalDevices_array);
        return (vkResult);
    }
    else if (physicalDeviceCount == 0)
    {
        LOGF("GetPhysicalDevice: Second call to vkEnumeratePhysicalDevices resulted into 0 physical devices");
        free(vkPhysicalDevices_array);
        return (vkResult);
    }
    else
    {
        LOGF("GetPhysicalDevice: Second call to vkEnumeratePhysicalDevices() succeded.");
    }

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        uint32_t queueCount = UINT32_MAX;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevices_array[i], &queueCount, NULL);

        // A physical device mustt have at least one queue property. But we are checking still for correctness
        if (queueCount > 0)
        {
            VkQueueFamilyProperties *vkQueueFamilyProperties_array = (VkQueueFamilyProperties *)malloc(queueCount * sizeof(VkQueueFamilyProperties));
            VkBool32 *isQueueSurfaceSupported_array = NULL;
            if (vkQueueFamilyProperties_array != NULL)
            {
                vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevices_array[i], &queueCount, vkQueueFamilyProperties_array);
                isQueueSurfaceSupported_array = (VkBool32*) malloc (queueCount * sizeof(VkBool32));
                if (isQueueSurfaceSupported_array != NULL)
                {
                    for (uint32_t j = 0; j < queueCount; ++j)
                    {
                        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevices_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);
                    }

                    // Print Queue families
#define STR(x) #x
#define PRINT_IF_SUPPORTED(queueType)                                                                               \
    if (isQueueSurfaceSupported_array[j] == VK_TRUE && (vkQueueFamilyProperties_array[j].queueFlags & queueType))   \
        LOGF("\t\t%s", STR(queueType));

                    LOGF("GetPhysicalDevice: Device %d", i);
                    for (uint32_t j = 0; j < queueCount; ++j)
                    {
                        LOGF("\tQueue (%d) : Flags => 0x%08X", j, vkQueueFamilyProperties_array[j].queueFlags);
                        PRINT_IF_SUPPORTED(VK_QUEUE_GRAPHICS_BIT);
                        PRINT_IF_SUPPORTED(VK_QUEUE_COMPUTE_BIT);
                        PRINT_IF_SUPPORTED(VK_QUEUE_TRANSFER_BIT);
                        PRINT_IF_SUPPORTED(VK_QUEUE_SPARSE_BINDING_BIT);
                        PRINT_IF_SUPPORTED(VK_QUEUE_PROTECTED_BIT);
                        PRINT_IF_SUPPORTED(VK_QUEUE_VIDEO_DECODE_BIT_KHR);
                        PRINT_IF_SUPPORTED(VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
                        PRINT_IF_SUPPORTED(VK_QUEUE_OPTICAL_FLOW_BIT_NV);
                    }

                    // Select if the physical device supports GRAPHICS_BIT
                    for (uint32_t j = 0; j < queueCount; ++j)
                    {
                        if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                            isQueueSurfaceSupported_array[j] == VK_TRUE)
                        {
                            vkPhysicalDevice_selected = vkPhysicalDevices_array[i];
                            graphicsQueueFamilyIndex_selected = j;
                            bFound = VK_TRUE;
                            break;
                        }
                    }
                    free(isQueueSurfaceSupported_array);
                    isQueueSurfaceSupported_array = NULL;
                }
                free(vkQueueFamilyProperties_array);
                vkQueueFamilyProperties_array = NULL;
            }
        }

        if (bFound == VK_TRUE)
        {
            break;
        }
    }

    if (bFound != VK_TRUE)
    {
        LOGF("GetPhysicalDevice: Unable to locate physical device with graphics properties");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;

        free(vkPhysicalDevices_array);
        vkPhysicalDevices_array = NULL;
        physicalDeviceCount = 0;

        return vkResult;
    }

    // Fill vkPhysicalMemoryProperties
    memset(&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);

    // Fill vkPhysicalDeviceFeatures
    memset(&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

    // Log supported features
#define LOG_IF_SUPPORTED(feature)                    \
    if (vkPhysicalDeviceFeatures.feature == VK_TRUE) \
        LOGF("\t%s", STR(feature));

    LOGF("GetPhysicalDevice: Features");
    LOG_IF_SUPPORTED(robustBufferAccess);
    LOG_IF_SUPPORTED(fullDrawIndexUint32);
    LOG_IF_SUPPORTED(imageCubeArray);
    LOG_IF_SUPPORTED(independentBlend);
    LOG_IF_SUPPORTED(geometryShader);
    LOG_IF_SUPPORTED(tessellationShader);
    LOG_IF_SUPPORTED(sampleRateShading);
    LOG_IF_SUPPORTED(dualSrcBlend);
    LOG_IF_SUPPORTED(logicOp);
    LOG_IF_SUPPORTED(multiDrawIndirect);
    LOG_IF_SUPPORTED(drawIndirectFirstInstance);
    LOG_IF_SUPPORTED(depthClamp);
    LOG_IF_SUPPORTED(depthBiasClamp);
    LOG_IF_SUPPORTED(fillModeNonSolid);
    LOG_IF_SUPPORTED(depthBounds);
    LOG_IF_SUPPORTED(wideLines);
    LOG_IF_SUPPORTED(largePoints);
    LOG_IF_SUPPORTED(alphaToOne);
    LOG_IF_SUPPORTED(multiViewport);
    LOG_IF_SUPPORTED(samplerAnisotropy);
    LOG_IF_SUPPORTED(textureCompressionETC2);
    LOG_IF_SUPPORTED(textureCompressionASTC_LDR);
    LOG_IF_SUPPORTED(textureCompressionBC);
    LOG_IF_SUPPORTED(occlusionQueryPrecise);
    LOG_IF_SUPPORTED(pipelineStatisticsQuery);
    LOG_IF_SUPPORTED(vertexPipelineStoresAndAtomics);
    LOG_IF_SUPPORTED(fragmentStoresAndAtomics);
    LOG_IF_SUPPORTED(shaderTessellationAndGeometryPointSize);
    LOG_IF_SUPPORTED(shaderImageGatherExtended);
    LOG_IF_SUPPORTED(shaderStorageImageExtendedFormats);
    LOG_IF_SUPPORTED(shaderStorageImageMultisample);
    LOG_IF_SUPPORTED(shaderStorageImageReadWithoutFormat);
    LOG_IF_SUPPORTED(shaderStorageImageWriteWithoutFormat);
    LOG_IF_SUPPORTED(shaderUniformBufferArrayDynamicIndexing);
    LOG_IF_SUPPORTED(shaderSampledImageArrayDynamicIndexing);
    LOG_IF_SUPPORTED(shaderStorageBufferArrayDynamicIndexing);
    LOG_IF_SUPPORTED(shaderStorageImageArrayDynamicIndexing);
    LOG_IF_SUPPORTED(shaderClipDistance);
    LOG_IF_SUPPORTED(shaderCullDistance);
    LOG_IF_SUPPORTED(shaderFloat64);
    LOG_IF_SUPPORTED(shaderInt64);
    LOG_IF_SUPPORTED(shaderInt16);
    LOG_IF_SUPPORTED(shaderResourceResidency);
    LOG_IF_SUPPORTED(shaderResourceMinLod);
    LOG_IF_SUPPORTED(sparseBinding);
    LOG_IF_SUPPORTED(sparseResidencyBuffer);
    LOG_IF_SUPPORTED(sparseResidencyImage2D);
    LOG_IF_SUPPORTED(sparseResidencyImage3D);
    LOG_IF_SUPPORTED(sparseResidency2Samples);
    LOG_IF_SUPPORTED(sparseResidency4Samples);
    LOG_IF_SUPPORTED(sparseResidency8Samples);
    LOG_IF_SUPPORTED(sparseResidency16Samples);
    LOG_IF_SUPPORTED(sparseResidencyAliased);
    LOG_IF_SUPPORTED(variableMultisampleRate);
    LOG_IF_SUPPORTED(inheritedQueries);

    if (vkPhysicalDeviceFeatures.tessellationShader == VK_TRUE)
    {
        LOGF("GetPhysicalDevice: Selected Physical device supports tesselation shader.");
    }
    else
    {
        LOGF("GetPhysicalDevice: Selected Physical device does not supports tesselation shader.");
    }

    if (vkPhysicalDeviceFeatures.geometryShader == VK_TRUE)
    {
        LOGF("GetPhysicalDevice: Selected Physical device supports geometry shader.");
    }
    else
    {
        LOGF("GetPhysicalDevice: Selected Physical device does not supports geometry shader.");
    }

    return vkResult;
}

VkResult PrintVkInfo(void)
{
    // Function declarations

    // Variable declarations
    VkResult vkResult = VK_SUCCESS;

    // Code
    LOGF("************ Vukan Information ***********");

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        LOGF("************* Device %d ***************", i);

        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset(&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        vkGetPhysicalDeviceProperties(vkPhysicalDevices_array[i], &vkPhysicalDeviceProperties);

        // API Version
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
        LOGF("ApiVersion: %d.%d.%d", majorVersion, minorVersion, patchVersion);
        // Device Name
        LOGF("Device Name: %s", vkPhysicalDeviceProperties.deviceName);
        // Driver Version
        uint32_t driverMajorVersion = VK_VERSION_MAJOR(vkPhysicalDeviceProperties.driverVersion);
        uint32_t driverMinorVersion = VK_VERSION_MINOR(vkPhysicalDeviceProperties.driverVersion);
        LOGF("Driver Version: %d.%d", driverMajorVersion, driverMinorVersion);
        // Device Tyoe
        switch (vkPhysicalDeviceProperties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                LOGF("Device Type: Integrated GPU (iGPU)");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                LOGF("Device Type: Discrete GPU (dGPU)");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                LOGF("Device Type: Virtual GPU (vGPU)");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                LOGF("Device Type: CPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                LOGF("Device Type: Other");
                break;
            default:
                LOGF("Device Type: UNKNOWN");
                break;
        }
        // VendorId
        LOGF("VendorId: 0x%04x", vkPhysicalDeviceProperties.vendorID);
        // DeviceId
        LOGF("DeviceId: 0x%04x", vkPhysicalDeviceProperties.deviceID);
    }
    LOGF("**************************************");

    // Free Physical Devices array
    free(vkPhysicalDevices_array);
    vkPhysicalDevices_array = NULL;

    return vkResult;
}

VkResult FillDeviceExtensionNames(void)
{
    // Variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t deviceExtensionCount = 0;
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    char **deviceExtensionNames_array = NULL;
    VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;

    // Step-1: Find how many device extenstions are supported by the vulkan driver of
    // this version and keep in local variable
    vkResult = vkEnumerateDeviceExtensionProperties(
        vkPhysicalDevice_selected,
        NULL,                  // pLayerName is NULL for global extensions
        &deviceExtensionCount, // pPropertyCount
        NULL);                 // pProperties
    if (vkResult != VK_SUCCESS)
    {
        LOGF("FillDeviceExtensionNames: First call to vkEnumerateDeviceExtensionProperties() failed with %i.", vkResult);
        return (vkResult);
    }
    else if (deviceExtensionCount == 0)
    {
        LOGF("FillDeviceExtensionNames: First call to vkEnumerateDeviceExtensionProperties() succeded with 0 exttensions");
        return (vkResult);
    }
    else
    {
        LOGF("FillDeviceExtensionNames: First call to vkEnumerateDeviceExtensionProperties() succeded with %d device extensions.", deviceExtensionCount);
    }

    // Step-2: Allocate and fill struct VkExtensionProperties array
    vkExtensionProperties_array = (VkExtensionProperties *)malloc(deviceExtensionCount * sizeof(VkExtensionProperties));
    if (vkExtensionProperties_array == NULL)
    {
        LOGF("FillDeviceExtensionNames: Unable to allocate VkExtensionProperties array");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    vkResult = vkEnumerateDeviceExtensionProperties(
        vkPhysicalDevice_selected,
        NULL,                         // pLayerName is NULL for global extensions
        &deviceExtensionCount,        // pPropertyCount
        vkExtensionProperties_array); // pProperties
    if (vkResult != VK_SUCCESS)
    {
        LOGF("FillDeviceExtensionNames: Second call to vkEnumerateDeviceExtensionProperties() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("FillDeviceExtensionNames: Second call to vkEnumerateDeviceExtensionProperties() succeded.");
    }

    // Step-3: Fill and display a local string array of extension names obtained from vkExtensionProperties_array
    deviceExtensionNames_array = (char **)malloc(deviceExtensionCount * sizeof(char *));
    if (deviceExtensionNames_array == NULL)
    {
        LOGF("FillDeviceExtensionNames: Unable to allocate deviceExtensionNames_array");
        free(vkExtensionProperties_array);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    LOGF("FillDeviceExtensionNames: Supported device extensions");
    for (uint32_t i = 0; i < deviceExtensionCount; ++i)
    {
        int extensionNameLength = strlen(vkExtensionProperties_array[i].extensionName) + 1;
        deviceExtensionNames_array[i] = (char *)malloc(extensionNameLength * sizeof(char));
        if (deviceExtensionNames_array[i] != NULL)
        {
            strcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName);
            deviceExtensionNames_array[i][extensionNameLength-1] = 0;
            LOGF("\t%s", deviceExtensionNames_array[i]);
        }
    }

    // Step-4 free vkExtensionProperties_array
    free(vkExtensionProperties_array);
    vkExtensionProperties_array = NULL;

    // Step-5: Find whether aboce extension names contain our required two surface extensions
    // and accordingly set
    for (uint32_t i = 0; i < deviceExtensionCount; ++i)
    {
        if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapChainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            break;
        }
    }

    // Step-6: Free allocated deviceExtensionNames_array
    for (uint32_t i = 0; i < deviceExtensionCount; ++i)
    {
        if (deviceExtensionNames_array[i] != NULL)
        {
            free(deviceExtensionNames_array[i]);
            deviceExtensionNames_array[i] = NULL;
        }
    }
    free(deviceExtensionNames_array);
    deviceExtensionNames_array = NULL;

    // Step-7: Display whether our vulkan driver supports our required extension names or not
    if (vulkanSwapChainExtensionFound == VK_FALSE)
    {
        LOGF("FillDeviceExtensionNames: Unable to locate VK_KHR_SWAPCHAIN_EXTENSION_NAME");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        LOGF("FillDeviceExtensionNames: Found VK_KHR_SWAPCHAIN_EXTENSION_NAME");
    }

    // Step-8: Print only supported device names
    for (uint32_t i = 0; i < enabledDeviceExtensionCount; ++i)
    {
        LOGF("FillDeviceExtensionNames: Enabled vulkan device extension name = %s", enabledDeviceExtensionNames_array[i]);
    }

    return vkResult;
}