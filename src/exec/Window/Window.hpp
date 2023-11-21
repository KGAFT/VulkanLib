#pragma once

#include "Monitor.hpp"
#include "WindowInputSystem.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>


class IWindowResizeCallback;

class Window {
private:
    static inline std::vector<Window *> windowInstances = std::vector<Window *>();

    static void resized(GLFWwindow *window, int width, int height);

public:
    static Window *createWindow(int width, int height, const char *title,
                                Monitor *monitor, bool addOpenglSupport);

private:
    Window(GLFWwindow *windowHandle, Monitor *attachMonitor, int width,
           int height, const char *title);

    GLFWwindow *windowHandle;
    Monitor *attachMonitor;
    int width;
    int height;
    const char *title;
    std::vector<IWindowResizeCallback *> resizeCallbacks;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    WindowInputSystem inputSystem;
    long long counter = 0;
    bool refreshInfo = false;
    double prevTime = 0;
public:
    bool needToClose();

    VkSurfaceKHR getWindowSurface(vk::Instance instance);

    GLFWwindow *getWindowHandle();

    void setMonitor(Monitor *monitor);

    int getWidth() const;

    int getHeight() const;



    void setSize(int width, int height);

    void setTitle(const char *title);

    const char *getTitle();

    void addResizeCallback(IWindowResizeCallback *resizeCallback);

    void removeResizeCallback(IWindowResizeCallback *resizeCallback);

    Monitor *getAttachMonitor() const;

    void preRenderEvents();

    void postRenderEvents();

    WindowInputSystem &getInputSystem();

    void enableRefreshRateInfo();

    void disableRefreshInfo();
private:
    void sizeCallbacks();

    void calculateRefreshData();
};

class IWindowResizeCallback {
public:
    virtual void resized(int width, int height) = 0;
};

