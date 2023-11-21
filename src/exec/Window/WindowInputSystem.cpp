#include "WindowInputSystem.hpp"

#include <iostream>

WindowInputSystem::WindowInputSystem(GLFWwindow *windowHandle)
    : windowHandle(windowHandle) {}

void WindowInputSystem::registerKeyCallback(IWindowKeyCallback *callback) {
  unsigned int keyCount = 0;
  WindowKey *cbKeys;
  keyCount = callback->getKeys(&cbKeys);
  time_t curTime;
  time(&curTime);
  for (unsigned int i = 0; i < keyCount; i++) {
    if (!isKeyExist(cbKeys[i].keyGlfwScanCode)) {
      InternalKey *key = new InternalKey;
      key->lastActiveTime = curTime;
      key->status = GLFW_RELEASE;
      key->scanCode = cbKeys[i].keyGlfwScanCode;
      key->isMouse =
          cbKeys[i].keyGlfwScanCode >= 0 && cbKeys[i].keyGlfwScanCode <= 7;
      this->keys.push_back(key);
    }
  }
  this->keyCallbacks.push_back(callback);
  std::cout << keyCallbacks.size() << std::endl;
}

void WindowInputSystem::registerMouseCallback(
    IWindowMouseMovementCallback *callback) {
  mouseCallbacks.push_back(callback);
}

void WindowInputSystem::setMode(WindowInputMode mode) { currentMode = mode; }

WindowInputSystem::~WindowInputSystem() {
  for (auto &el : keys) {
    delete el;
  }
}

void WindowInputSystem::checkKeyCallback() {
  unsigned int keyCount;
  WindowKey *cbKeys;
  for (IWindowKeyCallback *el : keyCallbacks) {
    processKeyCallback(el);
  }
}

void WindowInputSystem::checkMovementCallbacks() {
  double mousePosX, mousePosY = 0;
  glfwGetCursorPos(windowHandle, &mousePosX, &mousePosY);
  glm::vec2 curMousePos(mousePosX, mousePosY);
  if (curMousePos != oldMousePosition) {
    for (auto &el : mouseCallbacks) {
      if (el->getRequireWorkMode() == MODE_BOTH_MODES ||
          el->getRequireWorkMode() == currentMode) {
        el->moved(oldMousePosition, curMousePos);
      }
    }
  }
  if (currentMode == MODE_RETURN_INTO_CENTER) {
    int width, height = 0;
    glfwGetWindowSize(windowHandle, &width, &height);
    glfwSetCursorPos(windowHandle, width / 2, height / 2);
  }
  oldMousePosition = curMousePos;
}

void WindowInputSystem::processKeyCallback(IWindowKeyCallback *callback) {
  unsigned int keyCount = 0;
  WindowKey *cbKeys;
  keyCount = callback->getKeys(&cbKeys);
  InternalKey *key;
  for (unsigned int i = 0; i < keyCount; i++) {
    getKeyFromArray(cbKeys[i].keyGlfwScanCode, &key);
    int curStatus = key->isMouse
                        ? glfwGetMouseButton(windowHandle, key->scanCode)
                        : glfwGetKey(windowHandle, key->scanCode);
    time_t curTime;
    time(&curTime);
    switch (cbKeys[i].action) {
    case KEY_PRESSED:
      if (curStatus == GLFW_PRESS) {
        callback->keyPressed(&cbKeys[i]);
      }
      break;
    case KEY_CLICKED:
      if (key->status == GLFW_RELEASE && curStatus == GLFW_PRESS) {
        callback->keyPressed(&cbKeys[i]);
      }
      break;
    case KEY_HOLD:
      if (key->status == GLFW_PRESS && curStatus == GLFW_PRESS &&
          callback->getHoldDelay() <= (curTime - key->lastActiveTime) * 1000) {
        callback->keyPressed(&cbKeys[i]);
      }
      key->lastActiveTime = curTime;
      break;
    case KEY_RELEASED:
      if (key->status == GLFW_PRESS && curStatus == GLFW_RELEASE) {
        callback->keyPressed(&cbKeys[i]);
      }
      break;
    }
    key->status = curStatus;
    key->lastActiveTime =
        cbKeys[i].action == KEY_HOLD ? key->lastActiveTime : curTime;
  }
}

WindowInputMode WindowInputSystem::getMode() { return currentMode; }

bool WindowInputSystem::isKeyExist(int scanCode) {
  for (auto el : keys) {
    if (el->scanCode == scanCode) {
      return true;
    }
  }
  return false;
}

void WindowInputSystem::getKeyFromArray(int scanCode, InternalKey **pOutput) {
  for (auto el : keys) {
    if (el->scanCode == scanCode) {
      *pOutput = el;
      break;
    }
  }
}

void WindowInputSystem::removeKeyCallback(IWindowKeyCallback *callback) {
  int i = -1;
  for (unsigned int g = 0; g < keyCallbacks.size(); i++) {
    if (keyCallbacks[g] == callback) {
      i = g;
      break;
    }
  }
  if (i != -1) {
    keyCallbacks.erase(keyCallbacks.begin() + i);
  }
}

void WindowInputSystem::removeMouseCallback(
    IWindowMouseMovementCallback *callback) {
  int i = -1;
  for (unsigned int g = 0; g < mouseCallbacks.size(); i++) {
    if (mouseCallbacks[g] == callback) {
      i = g;
      break;
    }
  }
  if (i != -1) {
    mouseCallbacks.erase(mouseCallbacks.begin() + i);
  }
}
