#ifndef _DS3D_COMMON_ABI_WINDOW_H
#define _DS3D_COMMON_ABI_WINDOW_H

#include "ds3d/common/common.h"
#include "ds3d/common/abi_obj.h"
#include "ds3d/common/abi_dataprocess.h"

/**
 * @file maps data to a Window (display)
 */

namespace ds3d {

// ABI Interface for window system
class abiWindow {
public:
    // window closed callback
    typedef abiCallBackT<> CloseCB;
    // key pressed callback function(int key, int scancode, int action, int mods)
    typedef abiCallBackT<int, int, int, int> KeyPressCB;
    // framebuffer size changed callback function(int width, int height)
    typedef abiCallBackT<int, int> FbSizeChangedCB;

    // mouse cursor changed callback function(double xpos, double ypos)
    typedef abiCallBackT<double, double> MouseChangedCB;

    virtual ~abiWindow() = default;
    // get GLFWwindow raw pointer
    virtual void* getNativeWindow() = 0;
    // set close event callback
    virtual void setCloseCallback(const CloseCB* closeCb) = 0;
    // set key pressed event callback
    virtual void setKeyPressCallback(const KeyPressCB* keyCb) = 0;
    // set frame buffer size changed event callback
    virtual void setFbSizeChangedCallback(const FbSizeChangedCB* fbSizeChangedCb) = 0;
    // set mouse changed event callback
    virtual void setMouseChangedCallback(const MouseChangedCB* mouseChangedCb) = 0;
};

// raw reference pointer for abiWindow
typedef abiRefT<abiWindow> abiRefWindow;

}  // namespace ds3d

#endif  // _DS3D_COMMON_ABI_WINDOW_H