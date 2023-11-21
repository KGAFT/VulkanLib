#pragma once

#include <GLFW/glfw3.h>
#include <vector>


class MonitorMode{
    friend class Monitor;
private:
    const GLFWvidmode videoMode;
    MonitorMode(const GLFWvidmode &videoMode);
public:
    unsigned int getWidth();
    unsigned int getHeight();
    float getRefreshRate();
};

class Monitor
{
    friend class Window;
private:
    static inline std::vector<Monitor*> monitors = std::vector<Monitor*>();
public:
    static void enumerateMonitors(std::vector<Monitor*>& output);
private:
    Monitor(GLFWmonitor* monitor);
private:
    std::vector<MonitorMode*> videoModes;
    const char* name;
    GLFWmonitor* monitor;
    unsigned int widthMM;
    unsigned int heightMM;
    MonitorMode* currentMode = nullptr;
    GLFWwindow* attachWindow = nullptr;
public:
    const std::vector<MonitorMode*> &getVideoModes() const;
    const char *getName() const;
    unsigned int getWidthMM() const;
    unsigned int getHeightMM() const;
    void setMonitorMode(MonitorMode* mode);
};



