#pragma once

#include <cstdlib>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <ctime>

enum WindowInputMode{
    MODE_FREE_CURSOR = 0,
    MODE_RETURN_INTO_CENTER = 1,
    MODE_BOTH_MODES = 2
};

class IWindowMouseMovementCallback{
public:
    virtual void moved(glm::vec2 oldPos, glm::vec2 newPos)=0;
    virtual WindowInputMode getRequireWorkMode() = 0;
};
enum KeyAction{
    KEY_PRESSED = 0,
    KEY_CLICKED = 1,
    KEY_HOLD = 2,
    KEY_RELEASED = 3
};

struct WindowKey{
    int keyGlfwScanCode;
    KeyAction action;
};

struct InternalKey{
    int scanCode;
    int status;
    time_t lastActiveTime;
    bool isMouse;
};

class IWindowKeyCallback{
public:
    virtual void keyPressed(WindowKey* key) = 0;
    /**
                  @param output array output
                * @return amount of keys
                */
    virtual unsigned int getKeys(WindowKey** output) = 0;
    virtual size_t getHoldDelay() = 0;

};

class WindowInputSystem
{
    friend class Window;
public:
    WindowInputSystem(GLFWwindow *windowHandle);
private:
    std::vector<IWindowMouseMovementCallback*> mouseCallbacks;
    std::vector<IWindowKeyCallback*> keyCallbacks;
    glm::vec2 oldMousePosition;
    std::vector<InternalKey*> keys;
    GLFWwindow* windowHandle;
    WindowInputMode currentMode = MODE_FREE_CURSOR;
public:
    void registerKeyCallback(IWindowKeyCallback* callback);
    void registerMouseCallback(IWindowMouseMovementCallback* callback);
    void removeKeyCallback(IWindowKeyCallback* callback);
    void removeMouseCallback(IWindowMouseMovementCallback* callback);
    void setMode(WindowInputMode mode);
    WindowInputMode getMode();
    ~WindowInputSystem();
private:
    void checkKeyCallback();
    void checkMovementCallbacks();
    void getKeyFromArray(int scanCode, InternalKey** pOutput);
    void processKeyCallback(IWindowKeyCallback* callback);
    bool isKeyExist(int scanCode);

};


