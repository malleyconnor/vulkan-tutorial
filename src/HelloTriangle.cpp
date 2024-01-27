#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>

// Window width & height
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Whether or not we are running in debug mode
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


// Checking for use of validation layers and/or extensions
// =======================================================

std::vector<const char*> getRequiredExtensions() {
    // Get required extensions
    uint32_t glfwExtensionCount = 0;
    const char ** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // Add extension for custom debug callbacks
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// Checks if all the validation layers we want to use are available
bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Callback for processing errors/warnings/debug info however you want
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage <<  std::endl;

    return VK_FALSE;
}

// Initialize debug messenger to be used for createInstance and destroyInstance
// since the normal debug messenger works only on a valid instance.
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

// Load the destructor extension for the debugutils messenger and destroy that shit
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) 
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


// =======================================================



// Main application code
class HelloTriangleApplication {
    public:
        void run() {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }


    private:
        GLFWwindow *window; // acutal window
        VkInstance instance; // actual instance
        VkDebugUtilsMessengerEXT debugMessenger; //debug messenger (if applicable)

        void initWindow() {
            // Initialize glfw library
            glfwInit();

            // No OpenGL context
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            
            // No resizable window in this case
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            // Make window
            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        }


        void createInstance() {
            // Check for validation layer support
            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo{}; // app metadata
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo{}; // object metadata
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            // Get number of validation layers and their names
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else {
                createInfo.enabledLayerCount = 0;
            }

            // Use our function to get the extension names and add functionality for debug callbacks
            auto extensions = getRequiredExtensions();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            // Initialize the instance
            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

            // Create debug messenger (if applicable)
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else {
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr;
            }

            // See if the instance was successfully created
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }

        }


        // creates vulkan instance
        void initVulkan() {
            createInstance();
            setupDebugMessenger();
        }

        // Locates the extension for and creates the debug utils messenger
        void setupDebugMessenger() {
            if (!enableValidationLayers) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            createInfo.pfnUserCallback = debugCallback;

            // Load the debug utils messenger, throw a fit if that shit doesn't exist
            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        // main loop beep boop
        void mainLoop() {
            while(!glfwWindowShouldClose(window)) {
                glfwPollEvents();
            }
        }


        // Destroy all your shit
        void cleanup() {
            // If we're using validation layers, need to destroy the debug messenger
            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
                int a=0;
            }

            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();   
        }
};


// Main beep boop
int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}