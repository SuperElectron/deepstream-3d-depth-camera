#pragma once

#include "context.hpp"

using namespace ds3d;

namespace camera {
    class DepthCameraApp : public ds3d::app::Ds3dAppContext
    {
    public:
        DepthCameraApp() = default;
        ~DepthCameraApp();
        ErrCode initUserAppProfiling(const config::ComponentConfig &config);
        void setDataloaderSrc(gst::DataLoaderSrc src);
        void setDataRenderSink(gst::DataRenderSink sink);

        // configure action for FPS, timing, FileReader (ingest data) and FileWriter (dump data)
        AppProfiler &profiler(){ return _appProfiler; };

        void deinit();
        // stop pipeline and action objects for their elements
        ErrCode stop();

    private:

        bool busCall(GstMessage *msg) final;

    private:
        // placeholders for yaml file configurations (appsrc, appsink, and callback functions)
        gst::DataLoaderSrc _dataloaderSrc;
        gst::DataRenderSink _datarenderSink;
        AppProfiler _appProfiler;
    };
}

