// ************************************************************************* //
//
// Name		:   Nilesh Mahajan
// Roll No.	:   ARTR01-109
// Program	:   19-BuildCommandBuffer
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
// Device extensions related variables
uint32_t enabledDeviceExtensionCount = 0;
// VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char *enabledDeviceExtensionNames_array[1];
// Vulkan device
VkDevice vkDevice = VK_NULL_HANDLE;
// Device queue
VkQueue vkQueue = VK_NULL_HANDLE;
// Color format and color space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
// Present mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
// Swapchain related variables
int winWidth = WINDOW_WIDTH;
int winHeight = WINDOW_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;
// Swapchain Images and Image Views related variables
uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;
// Command pool related variable
VkCommandPool vkCommandPool = VK_NULL_HANDLE;
// Command buffers
VkCommandBuffer* vkCommandBuffer_array = NULL;
// Render Pass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;
// Framebuffers
VkFramebuffer *vkFrameBuffer_array = NULL;
// Semaphores
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;
// Fences
VkFence* vkFence_array = NULL;
// Build command buffer
// Clear color values
VkClearColorValue vkClearColorValue;

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
    VkResult CreateVulkanDevice(void);
    void GetDeviceQueue(void);
    VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult GetPhysicalDevicePresentMode(void);
    VkResult CreateSwapchain(VkBool32);
    VkResult CreateImagesAndImageViews(void);
    VkResult CreateCommandPool(void);
    VkResult CreateCommandBuffers(void);
    VkResult CreateRenderPass(void);
    VkResult CreateFramebuffers(void);
    VkResult CreateSemaphores(void);
    VkResult CreateFences(void);
    VkResult BuildCommandBuffers(void);

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

    // Create vulkan device
    vkResult = CreateVulkanDevice();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateVulkanDevice() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateVulkanDevice() succeded.");
    }

    // Get device queue associated with graphics family device queue at index 0
    GetDeviceQueue();

    // Get physical device surface format and color space
    vkResult = GetPhysicalDeviceSurfaceFormatAndColorSpace();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: GetPhysicalDeviceSurfaceFormatAndColorSpace() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: GetPhysicalDeviceSurfaceFormatAndColorSpace() succeded.");
    }

    // Get physical device present mode
    vkResult = GetPhysicalDevicePresentMode();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: GetPhysicalDevicePresentMode() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: GetPhysicalDevicePresentMode() succeded.");
    }

    // Create swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateSwapchain() failed with %i.", vkResult);

        // ----- IMP ------
        // We explicitly return hardcoded error since this function can return
        // value that is still success
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        LOGF("Initialize: CreateSwapchain() succeded.");
    }

    // Create images and imageViews
    vkResult = CreateImagesAndImageViews();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateImagesAndImageViews() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateImagesAndImageViews() succeded.");
    }

    // Create command pool
    vkResult = CreateCommandPool();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateCommandPool() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateCommandPool() succeded.");
    }

    // Create command buffers
    vkResult = CreateCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateCommandBuffers() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateCommandBuffers() succeded.");
    }

    // Create Render Pass
    vkResult = CreateRenderPass();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateRenderPass() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateRenderPass() succeded.");
    }

    // Create Framebuffers
    vkResult = CreateFramebuffers();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateFramebuffers() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateFramebuffers() succeded.");
    }

    // Create Semaphores
    vkResult = CreateSemaphores();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateSemaphores() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateSemaphores() succeded.");
    }

    // Create Fences
    vkResult = CreateFences();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: CreateFences() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: CreateFences() succeded.");
    }

    // Initialize clear color values analogous to glClearColor() to fill
    // blue color with solid color
    memset(&vkClearColorValue, 0, sizeof(VkClearColorValue));
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 1.0f;
    vkClearColorValue.float32[3] = 1.0f;

    // BuildCommandBuffers
    vkResult = BuildCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("Initialize: BuildCommandBuffers() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("Initialize: BuildCommandBuffers() succeded.");
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

    // No need to destroy/uninitialize device queue

    // Destroy vulkan Device
    if (vkDevice != VK_NULL_HANDLE)
    {
        LOGF("Uninitialize: waiting for all operations on the device to finish");

        // Wait for all operations on the device to finish
        VkResult vkResult = vkDeviceWaitIdle(vkDevice);

        // On success, this command returns
        //         VK_SUCCESS
        //  On failure, this command returns
        //         VK_ERROR_OUT_OF_HOST_MEMORY
        //         VK_ERROR_OUT_OF_DEVICE_MEMORY
        //         VK_ERROR_DEVICE_LOST
        LOGF("Uninitialize: vkDeviceWaitIdle returned %i", vkResult);

        // Destroy fences
        if (vkFence_array != NULL)
        {
            for (uint32_t i = 0; i < swapchainImageCount; ++i)
            {
                if (vkFence_array[i] != VK_NULL_HANDLE)
                {
                    // Wait for this fence
                    LOGF("Uninitialize: Waiting for fence %d", i);
                    vkResult = vkWaitForFences(vkDevice, 1, &vkFence_array[i], VK_TRUE, 0);
                    if (vkResult != VK_SUCCESS)
                    {
                        // TODO handle vkResult
                        LOGF("Uninitialize: Waiting for fence %d returned %d", i, vkResult);
                    }
                    LOGF("Uninitialize: Destroying fence %d", i);
                    vkDestroyFence(vkDevice, vkFence_array[i], NULL);
                    LOGF("Uninitialize: Destroyed fence %d", i);
                    vkFence_array[i] = VK_NULL_HANDLE;
                }
            }
        }

        // Destroy Semaphores
        if (vkSemaphore_renderComplete != VK_NULL_HANDLE)
        {
            LOGF("Uninitialize: Destroying vkSemaphore_renderComplete");
            vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
            LOGF("Uninitialize: vkSemaphore_renderComplete destroyed");
        }
        if (vkSemaphore_backBuffer != VK_NULL_HANDLE)
        {
            LOGF("Uninitialize: Destroying vkSemaphore_backBuffer");
            vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
            LOGF("Uninitialize: vkSemaphore_backBuffer destroyed");
        }

        if (vkFrameBuffer_array != NULL)
        {
            for (uint32_t i = 0; i < swapchainImageCount; ++i)
            {
                if (vkFrameBuffer_array[i] != NULL)
                {
                    LOGF("Uninitialize: Destroying frame buffer %d", i);
                    vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
                    LOGF("Uninitialize: Destroyed frame buffer %d", i);
                    vkFrameBuffer_array[i] = NULL;
                }
            }
            free(vkFrameBuffer_array);
            vkFrameBuffer_array = NULL;
            LOGF("Uninitialize: Freed vkFrameBuffer_array");
        }

        if (vkRenderPass != VK_NULL_HANDLE)
        {
            LOGF("Uninitialize: Destroying vkRenderPass");
            vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
            LOGF("Uninitialize: vkRenderPass destroyed");
            vkRenderPass = VK_NULL_HANDLE;
        }

        if (vkCommandBuffer_array != NULL)
        {
            for (uint32_t i = 0; i < swapchainImageCount; ++i)
            {
                LOGF("Uninitialize: Freeing command buffer %d", i);
                vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
            }

            LOGF("Uninitialize: Freeing vkCommandBuffer_array");
            free(vkCommandBuffer_array);
            vkCommandBuffer_array = NULL;
            LOGF("Uninitialize: vkCommandBuffer_array freed");
        }

        if (vkCommandPool != VK_NULL_HANDLE)
        {
            LOGF("Uninitialize: Destroying command pool");
            vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
            vkCommandPool = VK_NULL_HANDLE;
            LOGF("Uninitialize: Command pool destroyed");
        }

        // Destroy and free Image views
        if (swapchainImageView_array != NULL)
        {
            for (uint32_t i = 0; i < swapchainImageCount; ++i)
            {
                if (swapchainImageView_array[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
                    LOGF("Uninitialize: Destroyed swapchain image view %d", i);
                }
            }
            if (swapchainImageView_array)
            {
                free(swapchainImageView_array);
                swapchainImageView_array = NULL;
            }
            LOGF("Uninitialize: freed swapchainImageView_array");
        }

        // Destroy and free swapchain images
        if (swapchainImage_array != NULL)
        {
            // No need to destroy VkImage after destroying VkImageView
            // Since VkImage would be destroyed after destroying VkImageView
            LOGF("Uninitialize: No need to destroy VkImage after destroying VkImageView since destroying later would destroy VkImage");
            if (swapchainImage_array)
            {
                free(swapchainImage_array);
                swapchainImage_array = NULL;
            }
            LOGF("Uninitialize: freed swapchainImage_array");
        }

        // Destroy swapchain
        if (vkSwapchainKHR != VK_NULL_HANDLE)
        {
            LOGF("Uninitialize: destroying swapchain");
            vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
            LOGF("Uninitialize: swapchain destroyed");
            vkSwapchainKHR = VK_NULL_HANDLE;
        }

        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;

        LOGF("Uninitialize: vkDestroyDevice is done");
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

VkResult CreateVulkanDevice(void)
{
    // Function declarations
    VkResult FillDeviceExtensionNames(void);

    // Variable declarations
    VkResult vkResult = VK_SUCCESS;

    // Fill and initialize required device extension names and count global variables
    vkResult = FillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateVulkanDevice: FillDeviceExtensionNames() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateVulkanDevice: FillDeviceExtensionNames() succeded.");
    }

    // Set high priority for the proposed device queue family
    float queuePriorities[1];
    queuePriorities[0] = 1.0f;

    // Create queue on selected queue family at the point of device creation
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset(&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    // Create device
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset(&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledLayerCount = 0;
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    vkResult = vkCreateDevice(vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateVulkanDevice: vkCreateDevice() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateVulkanDevice: vkCreateDevice() succeded.");
    }
    return vkResult;
}

void GetDeviceQueue(void)
{
    LOGF("Selecting %d, %d from queue", graphicsQueueFamilyIndex_selected, 0);

    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);
    if (vkQueue != VK_NULL_HANDLE)
    {
        LOGF("GetDeviceQueue: vkGetDeviceQueue() returned valid vkQueue handle.");
    }
    else
    {
        LOGF("GetDeviceQueue: vkGetDeviceQueue() returned NULL for vkQueue.");
    }
}

const char *const GetColorSpaceString(VkColorSpaceKHR colorSpace)
{
#define STR(x) #x
#define GET_COLORSPACE_STR(x) \
    if (colorSpace == x)             \
        return STR(x);

    GET_COLORSPACE_STR(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_BT709_LINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_BT709_NONLINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_BT2020_LINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_HDR10_ST2084_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_DOLBYVISION_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_HDR10_HLG_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_PASS_THROUGH_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_DISPLAY_NATIVE_AMD);
    GET_COLORSPACE_STR(VK_COLOR_SPACE_DCI_P3_LINEAR_EXT);

    return "unknown";
}

const char* const GetSurfaceFormatString(VkFormat format)
{
#define STR(x) #x
#define GET_SURFACE_FORMAT_STR(x) \
    if (format == x)      \
        return STR(x);

    GET_SURFACE_FORMAT_STR(VK_FORMAT_UNDEFINED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R4G4_UNORM_PACK8);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R5G6B5_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B5G6R5_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B5G5R5A1_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A1R5G5B5_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8_SRGB);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8_SRGB);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8_SRGB);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8_SRGB);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R8G8B8A8_SRGB);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8A8_SRGB);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_UNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_SNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_USCALED_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_UINT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_SINT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8B8G8R8_SRGB_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2R10G10B10_SNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2R10G10B10_USCALED_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2R10G10B10_UINT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2R10G10B10_SINT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2B10G10R10_SNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2B10G10R10_USCALED_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2B10G10R10_UINT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A2B10G10R10_SINT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_SNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_USCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_SSCALED);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16B16A16_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32B32_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32B32_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32B32_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32B32A32_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32B32A32_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R32G32B32A32_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64B64_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64B64_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64B64_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64B64A64_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64B64A64_SINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R64G64B64A64_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_D16_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_X8_D24_UNORM_PACK32);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_D32_SFLOAT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_S8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_D16_UNORM_S8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_D24_UNORM_S8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_D32_SFLOAT_S8_UINT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC1_RGB_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC1_RGB_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC1_RGBA_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC2_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC2_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC3_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC3_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC4_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC4_SNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC5_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC5_SNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC6H_UFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC6H_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC7_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_BC7_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_EAC_R11_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_EAC_R11_SNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_EAC_R11G11_SNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_4x4_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_4x4_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x4_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x4_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x5_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x5_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x5_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x5_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x6_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x6_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x5_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x5_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x6_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x6_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x8_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x8_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x5_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x5_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x6_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x6_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x8_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x8_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x10_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x10_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x10_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x10_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x12_UNORM_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8B8G8R8_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8G8_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R10X6_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R10X6G10X6_UNORM_2PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R12X4_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R12X4G12X4_UNORM_2PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16B16G16R16_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B16G16R16G16_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A4R4G4B4_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A4B4G4R4_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A1B5G5R5_UNORM_PACK16);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8_UNORM);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_SFIXED5_NV);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8B8G8R8_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B8G8R8G8_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R10X6_UNORM_PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R12X4_UNORM_PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16B16G16R16_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_B16G16R16G16_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_R16G16_S10_5_NV);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR);
    GET_SURFACE_FORMAT_STR(VK_FORMAT_A8_UNORM_KHR);

    return "unknown";
}

VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // Function declarations
    const char *const GetColorSpaceString(VkColorSpaceKHR colorSpace);
    const char *const GetSurfaceFormatString(VkFormat format);

    // Variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    // Get the count of supported surface color formats
    uint32_t vkSurfaceFormatCount = 0;
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceFormatCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: First call to vkGetPhysicalDeviceSurfaceFormatsKHR() failed with %i.", vkResult);
        return (vkResult);
    }
    else if (vkSurfaceFormatCount == 0)
    {
        LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: First call to vkGetPhysicalDeviceSurfaceFormatsKHR() returned 0 formats");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: First call to vkGetPhysicalDeviceSurfaceFormatsKHR() succeded with %d count", vkSurfaceFormatCount);
    }

    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(vkSurfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    if (vkSurfaceFormatKHR_array == NULL)
    {
        LOGF("Failed to allocate vkSurfaceFormatKHR_array");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceFormatCount, vkSurfaceFormatKHR_array);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Second call to vkGetPhysicalDeviceSurfaceFormatsKHR() failed with %i.", vkResult);
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        return (vkResult);
    }
    else if (vkSurfaceFormatCount == 0)
    {
        LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Second call to vkGetPhysicalDeviceSurfaceFormatsKHR() returned 0 formats");
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Second call to vkGetPhysicalDeviceSurfaceFormatsKHR() succeded with %d count", vkSurfaceFormatCount);
    }

    // // Print supported surface count
    // LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Supported surface formats along with color space");
    // for (uint32_t i = 0; i < vkSurfaceFormatCount; ++i)
    // {
    //     LOGF("\t%s ------- %s", GetSurfaceFormatString(vkSurfaceFormatKHR_array[i].format), GetColorSpaceString(vkSurfaceFormatKHR_array[i].colorSpace));
    // }

    // Decide the surface color format and colorspace
    if (vkSurfaceFormatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
    {
        vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
    }

    // Decide the color space
    vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

    LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Selected surface format %s", GetSurfaceFormatString(vkFormat_color));
    LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Selected color space %s", GetColorSpaceString(vkColorSpaceKHR));

    // Free vkSurfaceFormatKHR_array
    free(vkSurfaceFormatKHR_array);
    vkSurfaceFormatKHR_array = NULL;
    LOGF("GetPhysicalDeviceSurfaceFormatAndColorSpace: Freed vkSurfaceFormatKHR_array");

    return vkResult;
}

const char *const GetPhysicalDevicePresentModeString(VkPresentModeKHR mode)
{
#define STR(x) #x
#define GET_PRESENT_MODE_STR(x) \
    if (mode == x)              \
        return STR(x);

    GET_PRESENT_MODE_STR(VK_PRESENT_MODE_IMMEDIATE_KHR);
    GET_PRESENT_MODE_STR(VK_PRESENT_MODE_MAILBOX_KHR);
    GET_PRESENT_MODE_STR(VK_PRESENT_MODE_FIFO_KHR);
    GET_PRESENT_MODE_STR(VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    GET_PRESENT_MODE_STR(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
    GET_PRESENT_MODE_STR(VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);

    return "unknown";
}

VkResult GetPhysicalDevicePresentMode(void)
{
    // Function declarations
    const char *const GetPhysicalDevicePresentModeString(VkColorSpaceKHR colorSpace);

    // Variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t presentModeCount = 0;

    // Code
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetPhysicalDevicePresentMode: First call to vkGetPhysicalDeviceSurfacePresentModesKHR() failed with %i.", vkResult);
        return (vkResult);
    }
    else if (presentModeCount == 0)
    {
        LOGF("GetPhysicalDevicePresentMode: First call to vkGetPhysicalDeviceSurfacePresentModesKHR() returned 0 present modes");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        LOGF("GetPhysicalDevicePresentMode: First call to vkGetPhysicalDeviceSurfacePresentModesKHR() succeded with %d count", presentModeCount);
    }

    VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    if (vkPresentModeKHR_array == NULL)
    {
        LOGF("Failed to allocate vkPresentModeKHR_array");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("GetPhysicalDevicePresentMode: Second call to vkGetPhysicalDeviceSurfacePresentModesKHR() failed with %i.", vkResult);
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        return (vkResult);
    }
    else if (presentModeCount == 0)
    {
        LOGF("GetPhysicalDevicePresentMode: Second call to vkGetPhysicalDeviceSurfacePresentModesKHR() returned 0 formats");
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        LOGF("GetPhysicalDevicePresentMode: Second call to vkGetPhysicalDeviceSurfacePresentModesKHR() succeded");
    }

    // // Print supported present modes
    // LOGF("GetPhysicalDevicePresentMode: Supported present modes");
    // for (uint32_t i = 0; i < presentModeCount; ++i)
    // {
    //     LOGF("\t%s", GetPhysicalDevicePresentModeString(vkPresentModeKHR_array[i]));
    // }

    // Select present mode
    for (uint32_t i = 0; i < presentModeCount; ++i)
    {
        if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    // Select FIFO mode if mailbox is not available
    if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
    }

    LOGF("GetPhysicalDevicePresentMode: Selected present mode %s", GetPhysicalDevicePresentModeString(vkPresentModeKHR));

    // Free vkPresentModeKHR_array
    free(vkPresentModeKHR_array);
    vkPresentModeKHR_array = NULL;
    LOGF("GetPhysicalDevicePresentMode: Freed vkPresentModeKHR_array");

    return (vkResult);
}

VkResult CreateSwapchain(VkBool32 vSync)
{
    // Function declarations
    VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult GetPhysicalDevicePresentMode(void);

    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Code

    // Get physical device surface format and color space
    vkResult = GetPhysicalDeviceSurfaceFormatAndColorSpace();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateSwapchain: GetPhysicalDeviceSurfaceFormatAndColorSpace() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateSwapchain: GetPhysicalDeviceSurfaceFormatAndColorSpace() succeded.");
    }

    // Get physical device capabilities for our surface
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset(&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vkPhysicalDevice_selected,
        vkSurfaceKHR,
        &vkSurfaceCapabilitiesKHR);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateSwapchain: vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateSwapchain: vkGetPhysicalDeviceSurfaceCapabilitiesKHR() succeded.");
    }

    // Find out desired swapchain image count
    uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    uint32_t desiredNumberOfSwapchainImages = 0;

    if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
    {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }

    // Step 4: Choose size of the swapchain image
    memset(&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

        LOGF("CreateSwapchain: vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX");
    }
    else
    {
        LOGF("CreateSwapchain: vkSurfaceCapabilitiesKHR.currentExtent.width == UINT32_MAX");

        // if surface size is already defined, then swapchain image size must match with it
        VkExtent2D vkExtent2D;  
        memset(&vkExtent2D, 0, sizeof(VkExtent2D));
        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        vkExtent2D_swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width,
                                         min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height,
                                          min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
    }
    LOGF("CreateSwapchain: Swapchain image width X height = %d X %d", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);

    // Step5: Set swapchain image usage flag
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is required.
    // Although VK_IMAGE_USAGE_TRANSFER_SRC_BIT is not required for triangle application, it maybe useful
    // for texture/FBO/compute hence mentioning here
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // Step6: Whether to consider pre-transform/flipping or not
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
    if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }

    // Step7: Get physical device present mode
    vkResult = GetPhysicalDevicePresentMode();
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateSwapchain: GetPhysicalDevicePresentMode() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateSwapchain: GetPhysicalDevicePresentMode() succeded.");
    }

    // Step8: Initialize VkSwapchainCreateInfoKHR structure
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset(&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateSwapchain: vkCreateSwapchainKHR() failed with %i.", vkResult);
        return (vkResult);
    }
    else
    {
        LOGF("CreateSwapchain: vkCreateSwapchainKHR() succeded.");
    }

    return vkResult;
}

VkResult CreateImagesAndImageViews(void)
{
    // Function declarations

    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Get swapchain image count
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateImagesAndImageViews: first call to vkGetSwapchainImagesKHR() failed with %i.", vkResult);
        return (vkResult);
    }
    else if (swapchainImageCount == 0)
    {
        LOGF("CreateImagesAndImageViews: first call to vkGetSwapchainImagesKHR() returned 0 image count.");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        LOGF("CreateImagesAndImageViews: first call to vkCreateSwapchainKHR() succeded with %d images", swapchainImageCount);
    }

    // Allocate memory
    swapchainImage_array = (VkImage*) malloc(swapchainImageCount * sizeof(VkImage));
    if (swapchainImage_array == NULL)
    {
        LOGF("CreateImagesAndImageViews: failed to allocate swapchainImage_array.");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    // Fill this array by swapchain images
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateImagesAndImageViews: second call to vkGetSwapchainImagesKHR() failed with %i.", vkResult);
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        return (vkResult);
    }
    else if (swapchainImageCount == 0)
    {
        LOGF("CreateImagesAndImageViews: second call to vkGetSwapchainImagesKHR() returned 0 image count.");
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        LOGF("CreateImagesAndImageViews: second call to vkCreateSwapchainKHR() succeded with %d images", swapchainImageCount);
    }

    // allocate array of swapchain image views
    swapchainImageView_array = (VkImageView *)malloc(swapchainImageCount * sizeof(VkImageView));
    if (swapchainImageView_array == NULL)
    {
        LOGF("CreateImagesAndImageViews: failed to allocate swapchainImageView_array.");
        return (VK_ERROR_INITIALIZATION_FAILED);
    }

    // Initialize VkImageViewCreateInfo structure
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset(&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color;
    // VkComponentMapping
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R; // type enum VkComponentSwizzle
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    // VkImageSubresourceRange
    // AspectMask: which part of the image is affected by image barrier
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    //
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    //
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    // 
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // Fill Image view array using above struct
    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[i];
        vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("CreateImagesAndImageViews: vkCreateImageView failed for iteration %d with %d", i, vkResult);
            break;
        }
        else
        {
            LOGF("CreateImagesAndImageViews: vkCreateImageView succeded for iteration %d", i);
        }
    }

    // Code
    return vkResult;
}

VkResult CreateCommandPool(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
    memset(&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

    vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateCommandPool: vkCreateCommandPool failed with %d", vkResult);
        return vkResult;
    }
    else if (vkCommandPool == VK_NULL_HANDLE)
    {
        LOGF("CreateCommandPool: vkCreateCommandPool returned NULL command pool handle");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return vkResult;
}

VkResult CreateCommandBuffers(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset(&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkCommandBuffer_array = (VkCommandBuffer*) malloc(swapchainImageCount * sizeof(VkCommandBuffer));
    if (vkCommandBuffer_array == NULL)
    {
        LOGF("CreateCommandBuffers: Failed to allocate vkCommandBufferAllocateInfo");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("CreateCommandBuffers: vkAllocateCommandBuffers for %d failed with %d", i, vkResult);
            return vkResult;
        }
    }

    return vkResult;
}

VkResult CreateRenderPass(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Declare and initialize VkAttachmentDescription structure array (Array depends upon number of attachments)
    // Although we have only 1 attachment (color attachment) in this example, we will consider it as an array
    VkAttachmentDescription vkAttachmentDescription_array[1];
    memset(
        vkAttachmentDescription_array,
        0,
        sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
    vkAttachmentDescription_array[0].flags = 0;
    // This can be useful in embeded to use same memory by aliasing if we use VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    // Specifying how the contents of color and depth components of the attachment are treated at the beginning of the subpass where it is first used
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // How the contents of color and depth components of the attachment are treated at the end of the subpass where it is last used.
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // how the contents of stencil components of the attachment are treated at the beginning of the subpass where it is first used.
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // how the contents of stencil components of the attachment are treated at the end of the last subpass where it is used.
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // layout the attachment image subresource will be in when a render pass instance begins.
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // layout the attachment image subresource will be transitioned to when a render pass instance ends.
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 2) Declare and initialize VkAttachmentReference structure which will have information about the attachment described above.
    VkAttachmentReference vkAttachmentReference;
    memset(&vkAttachmentReference, 0, sizeof(VkAttachmentReference));
    // Index of the attachment in vkAttachmentDescription_array
    vkAttachmentReference.attachment = 0;
    // specifies a layout that must only be used with attachment accesses in the graphics pipeline.
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Declare and initialize VkSubpassDescription structure and keep information about above VkAttachmentReference structure.
    VkSubpassDescription vkSubpassDescription;
    memset(&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = NULL;
    vkSubpassDescription.pPreserveAttachments = NULL;
    vkSubpassDescription.preserveAttachmentCount = 0;

    // Declare and initialize VkRenderPassCreateInfo and refer VkAttachmentDescription and VkSubpassDescription into it.
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset(&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;

    // Now call VkCreateRenderpass() to create actual renderpass.
    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);

    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateRenderPass: vkCreateRenderPass failed with %d", vkResult);
        return vkResult;
    }
    else if (vkRenderPass == VK_NULL_HANDLE)
    {
        LOGF("CreateRenderPass: vkCreateRenderPass returned NULL render pass handle");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return vkResult;
}

VkResult CreateFramebuffers(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    VkImageView vkImageView_attachments_array[1];
    memset(vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

    VkFramebufferCreateInfo vkFramebufferCreateInfo;
    memset(&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.pNext = NULL;
    vkFramebufferCreateInfo.flags = 0;
    vkFramebufferCreateInfo.renderPass = vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
    vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
    vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
    vkFramebufferCreateInfo.layers = 1;

    vkFrameBuffer_array = (VkFramebuffer*) malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    if (vkFrameBuffer_array == NULL)
    {
        LOGF("CreateFramebuffers: Failed to allocate vkFrameBuffer_array");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    memset(vkFrameBuffer_array, 0, sizeof(VkFramebuffer) * swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkImageView_attachments_array[0] = swapchainImageView_array[i];

        LOGF("CreateFramebuffers: Creating frame buffer %d", i);
        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFrameBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("CreateFramebuffers: Failed to create framebuffer %d. VkResult = %d", i, vkResult);
            return vkResult;
        }
        LOGF("CreateFramebuffers: Created frame buffer %d", i);
    }

    return vkResult;
}

VkResult CreateSemaphores(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset(&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;
    vkSemaphoreCreateInfo.flags = 0;   // reserved and must be zero

    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backBuffer);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateSemaphores: Failed to create vkSemaphore_backBuffer. VkResult = %d", vkResult);
        return vkResult;
    }
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_renderComplete);
    if (vkResult != VK_SUCCESS)
    {
        LOGF("CreateSemaphores: Failed to create vkSemaphore_renderComplete. VkResult = %d", vkResult);
        return vkResult;
    }

    return vkResult;
}

VkResult CreateFences(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    if (vkFence_array != NULL)
    {
        LOGF("CreateFences: vkFence_array is not NULL");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Allocate and initialize VkFence array
    vkFence_array = (VkFence*) malloc(sizeof(VkFence) * swapchainImageCount);
    if (vkFence_array == NULL)
    {
        LOGF("CreateFences: failed to allocate vkFence_array");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkFence_array[i] = VK_NULL_HANDLE;
    }

    // Create signalled fences
    VkFenceCreateInfo vkFenceCreateInfo;
    memset(&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // can be 0 to create non-signalled fence

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("CreateFences: Failed to allocate %dth fence. VkResult = %d", i, vkResult);
            break;
        }
    }

    return vkResult;
}

VkResult BuildCommandBuffers(void)
{
    // Variables
    VkResult vkResult = VK_SUCCESS;

    // Loop per swapchain images
    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        // Reset command buffers
        // Specifying 0 means that don't release the resources created by the command pool for this
        // buffer. The command buffer is created from command pool
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("BuildCommandBuffers: Failed to reset %dth command buffer. VkResult = %d", i, vkResult);
            break;
        }
        else
        {
            LOGF("BuildCommandBuffers: Resetted %dth command buffer.", i);
        }

        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset(&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.pInheritanceInfo = NULL;
        vkCommandBufferBeginInfo.flags = 0; // we're not going to use this command buffer between multiple threads
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("BuildCommandBuffers: vkBeginCommandBuffer failed for %dth command buffer. VkResult = %d", i, vkResult);
            break;
        }
        else
        {
            LOGF("BuildCommandBuffers: Begin %dth command buffer.", i);
        }

        // Set clear values
        VkClearValue vkClearValue_array[1];
        memset(vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));
        vkClearValue_array[0].color = vkClearColorValue;

        // Begin render pass
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset(&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFrameBuffer_array[i];

        LOGF("BuildCommandBuffers: vkCmdBeginRenderPass %dth command buffer.", i);

        // VK_SUBPASS_CONTENTS_INLINE specifies that the contents of the subpass will be recorded inline in
        // the primary command buffer, and secondary command buffers must not be executed within the subpass.
        vkCmdBeginRenderPass(vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Here, we should call vulkan drawing functions

        // End render pass
        LOGF("BuildCommandBuffers: vkCmdEndRenderPass %dth command buffer.", i);
        vkCmdEndRenderPass(vkCommandBuffer_array[i]);

        // End command buffer recording
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            LOGF("BuildCommandBuffers: vkEndCommandBuffer failed for %dth command buffer. VkResult = %d", i, vkResult);
            break;
        }
        else
        {
            LOGF("BuildCommandBuffers: vkEndCommandBuffer %dth command buffer.", i);
        }
    }

    return vkResult;
}