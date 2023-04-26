#ifndef NVDS3D_CUSTOMLIB_FACTORY_HPP
#define NVDS3D_CUSTOMLIB_FACTORY_HPP

#include <dlfcn.h>
#include "common.h"
#include <errno.h>

#include <functional>
#include <iostream>

/**
 * @file enables loading of custom lib as defined in yaml file
 */
namespace ds3d {

template <class T> T* dlsym_ptr(void* handle, char const* name)
{
    return reinterpret_cast<T*>(dlsym(handle, name));
}

class CustomLibFactory {
public:
    CustomLibFactory() = default;

    ~CustomLibFactory()
    {
        if (_libHandle) {
            dlclose(_libHandle);
        }
    }

    template <class CustomRefCtx>
    CustomRefCtx* CreateCtx(const std::string& libName, const std::string& symName)
    {
        if (!_libHandle) {
            _libName = libName;
            _libHandle = dlopen(libName.c_str(), RTLD_NOW | RTLD_LOCAL);
        } else {
            DS3D_FAILED_RETURN(
                _libName == libName || libName.empty(), nullptr,
                "CustomLibFactory existing libname: %s is different from new lib: %s",
                _libName.c_str(), libName.c_str());
        }
        DS3D_FAILED_RETURN(
            _libHandle, nullptr, "open custome-lib: %s failed. dlerr: %s", _libName.c_str(),
            dlerror()
        );
        LOG_INFO("Library Opened Successfully");

        std::function<CustomRefCtx*()> createCtxFunc =
            dlsym_ptr<CustomRefCtx*()>(_libHandle, symName.c_str());
        DS3D_FAILED_RETURN(createCtxFunc, nullptr, "dlsym: %s not found, error: %s", symName.c_str(), dlerror());

        CustomRefCtx* customCtx = createCtxFunc();
        DS3D_FAILED_RETURN(
            customCtx, nullptr, "create custom context failed during call: %s", symName.c_str()
        );
        LOG_INFO("Custom Context created from %s", symName.c_str());
        return customCtx;
    }

public:
    void* _libHandle = nullptr;
    std::string _libName;
};

}  // namespace 3d

#endif
