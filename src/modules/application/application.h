#pragma once
#include <cuda_runtime_api.h>

#include "application.h"
#include "context.hpp"
#include "camera.h"

#undef CHECK_ERROR
#define CHECK_ERROR(statement, fmt, ...) DS3D_FAILED_RETURN(statement, -1, fmt, ##__VA_ARGS__)

#undef RETURN_ERROR
#define RETURN_ERROR(statement, fmt, ...) DS3D_ERROR_RETURN(statement, fmt, ##__VA_ARGS__)

using namespace ds3d;
using namespace camera;
using ConfigList = std::vector<config::ComponentConfig>;

/** Set global gAppCtx */
static std::weak_ptr <camera::DepthCameraApp> gAppCtx;

/**
 * @brief safe close of display window rendered by appsink and terminate the gstreamer pipeline
 */
void static WindowClosed()
{
    LOG_DEBUG("Window closed.");
    auto appCtx = gAppCtx.lock();

    if (!appCtx)
    {
        return;
    }

    if (appCtx->isRunning(5000))
    {
        appCtx->sendEOS();
    } else
    {
        appCtx->quitMainLoop();
    }
};

/**
 * @brief handle program interrupt signal. It installs default handler after handling the interrupt.
 * @param signum an integer to hold exit value
 */
void static intr_handler(int signum)
{
    LOG_INFO("User Interrupted..");

    ShrdPtr <DepthCameraApp> appCtx = gAppCtx.lock();
    if (appCtx)
    {
        if (appCtx->isRunning())
        {
            appCtx->sendEOS();
        } else
        {
            appCtx->quitMainLoop();
        }
    } else
    {
        LOG_ERROR("program terminated.");
        std::terminate();
    }
};

/**
 * @brief install custom handler for program interrupt signal.
 * @param intr_handler the function you want to call to terminate the executable (clean up and safe exit for Crtl+C)
 */
static void intr_setup(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = intr_handler;
    sigaction(SIGINT, &action, NULL);
};

namespace myApp
{

class Application
{
public:
    Application();

//    void WindowClosed();
    void help(const char *bin);
    void intr_handler(int signum);
    ErrCode CreateLoaderSource(
            std::map <config::ComponentType,
            ConfigList> &configTable,
            gst::DataLoaderSrc &loaderSrc,
            bool startLoader);
    ErrCode CreateRenderSink(
            std::map <config::ComponentType,
            ConfigList> &configTable,
            gst::DataRenderSink &renderSink,
            bool startRender);
    ~Application();

};
} // namespace exec

