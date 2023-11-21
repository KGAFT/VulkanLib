#include <vulkan/vulkan.h>
#include "Window.hpp"
#include <stdexcept>

void Window::resized(GLFWwindow *window, int width, int height) {
    for (auto &el: windowInstances) {
        if (el->windowHandle == window) {
            el->width = width;
            el->height = height;
            el->sizeCallbacks();
        }
    }
}

Window *Window::createWindow(int width, int height, const char *title,
                             Monitor *monitor, bool addOpenglSupport) {
    if (!addOpenglSupport) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window) {
        if (monitor) {
            glfwSetWindowMonitor(window, monitor->monitor, 0, 0,
                                 monitor->currentMode->getWidth(),
                                 monitor->currentMode->getHeight(),
                                 monitor->currentMode->getRefreshRate());
            monitor->attachWindow = window;
        }

        Window *res = new Window(
                window, monitor, monitor ? monitor->currentMode->getWidth() : width,
                monitor ? monitor->currentMode->getHeight() : height, title);
        glfwSetWindowSizeCallback(window, resized);
        windowInstances.push_back(res);
        return res;
    }
    throw std::runtime_error("Failed to create window");
}

Window::Window(GLFWwindow *windowHandle, Monitor *attachMonitor, int width,
               int height, const char *title)
        : windowHandle(windowHandle), attachMonitor(attachMonitor), width(width),
          height(height), title(title), inputSystem(windowHandle) {}

WindowInputSystem &Window::getInputSystem() { return inputSystem; }

Monitor *Window::getAttachMonitor() const { return attachMonitor; }

void Window::preRenderEvents() {
    inputSystem.checkMovementCallbacks();
    inputSystem.checkKeyCallback();
}

void Window::postRenderEvents() {
    glfwPollEvents();
    if(refreshInfo){
        calculateRefreshData();
    }
}

void Window::sizeCallbacks() {
    for (auto &el: resizeCallbacks) {
        el->resized(width, height);
    }
}

bool Window::needToClose() { return glfwWindowShouldClose(windowHandle); }

VkSurfaceKHR Window::getWindowSurface(vk::Instance instance) {
    if (surface == VK_NULL_HANDLE) {
        glfwCreateWindowSurface(instance, windowHandle, nullptr, &surface);
    }
    return surface;
}

GLFWwindow *Window::getWindowHandle() { return windowHandle; }

void Window::setMonitor(Monitor *monitor) {
    if (this->attachMonitor != nullptr) {
        attachMonitor->attachWindow = nullptr;
    }
    attachMonitor = monitor;
    glfwSetWindowMonitor(windowHandle, monitor->monitor, 0, 0,
                         monitor->currentMode->getWidth(),
                         monitor->currentMode->getHeight(),
                         monitor->currentMode->getRefreshRate());
}

int Window::getWidth() const { return width; }

int Window::getHeight() const { return height; }

void Window::setSize(int width, int height) {
    if (attachMonitor != nullptr) {
        throw std::runtime_error("Failed to resize window, window attached to "
                                 "monitor, change monitor video mode instead");
    }
    this->width = width;
    this->height = height;
    glfwSetWindowSize(windowHandle, width, height);
}

void Window::setTitle(const char *title) {
    this->title = title;
    glfwSetWindowTitle(windowHandle, title);
}

const char *Window::getTitle() { return title; }

void Window::addResizeCallback(IWindowResizeCallback *resizeCallback) {
    resizeCallbacks.push_back(resizeCallback);
}

void Window::removeResizeCallback(IWindowResizeCallback *resizeCallback) {
    int removeIndex = -1;
    for (int i = 0; i < resizeCallbacks.size(); i++) {
        if (resizeCallbacks[i] == resizeCallback) {
            removeIndex = i;
        }
    }

    if (removeIndex != -1) {
        resizeCallbacks.erase(resizeCallbacks.begin() + removeIndex);
    }
}

void Window::enableRefreshRateInfo() {
    refreshInfo = true;
}

void Window::calculateRefreshData() {
    double crntTime = glfwGetTime();
    double timeDiff = crntTime - prevTime;
    counter++;

    if (timeDiff >= 1.0 / 30.0) {
        std::string FPS = std::to_string((int) ((1.0 / timeDiff) * counter));
        std::string ms = std::to_string((timeDiff / counter) * 1000);
        glfwSetWindowTitle(windowHandle, (std::string(title) + " FPS: " + FPS + " ms: " + ms).c_str());

        prevTime = crntTime;
        counter = 0;

    }
}

void Window::disableRefreshInfo() {
    refreshInfo = false;
}

