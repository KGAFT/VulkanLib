#include "Monitor.hpp"

#include <stdexcept>


unsigned int MonitorMode::getWidth()
{
    return videoMode.width;
}

unsigned int MonitorMode::getHeight()
{
    return videoMode.height;
}

float MonitorMode::getRefreshRate()
{
    return videoMode.refreshRate;
}

MonitorMode::MonitorMode(const GLFWvidmode& videoMode) : videoMode(videoMode)
{}

void Monitor::enumerateMonitors(std::vector<Monitor*>& output)
{
    if(monitors.size()==0){
        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        for(int i = 0; i<count; i++){
            Monitor::monitors.push_back(new Monitor(monitors[i]));
            output.push_back(Monitor::monitors[Monitor::monitors.size()-1]);
        }
    }
    else{
        for(auto& el : monitors){
            output.push_back(el);
        }
    }

}

Monitor::Monitor(GLFWmonitor *monitor) : monitor(monitor)
{
    name = glfwGetMonitorName(monitor);
    glfwGetMonitorPhysicalSize(monitor, (int*)&widthMM, (int*)&heightMM);
    int count = 0;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);
    for(int i = 0; i<count; i++){
        videoModes.push_back(new MonitorMode(modes[i]));
    }
    currentMode = videoModes[videoModes.size()-1];

}

const std::vector<MonitorMode*> &Monitor::getVideoModes() const
{
    return videoModes;
}

const char *Monitor::getName() const
{
    return name;
}

unsigned int Monitor::getWidthMM() const
{
    return widthMM;
}

unsigned int Monitor::getHeightMM() const
{
    return heightMM;
}

void Monitor::setMonitorMode(MonitorMode *mode)
{
    bool found = false;
    for(auto& el : videoModes){
        if(el==mode){
            found = true;
            break;
        }
    }
    if(!found){
        throw std::runtime_error("No such vide mode");
    }
    currentMode  = mode;
    if(attachWindow){
        glfwSetWindowMonitor(attachWindow, monitor, 0, 0, currentMode->getWidth(), currentMode->getHeight(), currentMode->getRefreshRate());
    }
}

