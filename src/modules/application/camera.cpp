#include "camera.h"

using namespace camera;

/**
 * @brief clears all objects to avoid memory leaks
 */
DepthCameraApp::~DepthCameraApp() { deinit(); }

/**
 * @brief parse fields from config file (3dDs::userapp)
 * @param config `struct ComponentConfig` from 3d::common::config.h that holds memory types for yaml field entries
 * @return ErrCode for flow handling
 */
ErrCode DepthCameraApp::initUserAppProfiling(const config::ComponentConfig &config)
{
    auto initP = [this, &config]() {
        return _appProfiler.initProfiling(config);
    };
    DS3D_ERROR_RETURN(config::CatchYamlCall(initP), "parse 3dDs::userapp failed");
    return ErrCode::kGood;
}

/**
 * @brief configure action for appsrc (type: ds3d::dataloader from yaml)
 * @param src pass ownership of the gstreamer element to the application layer (from header instantiation)
 */
void DepthCameraApp::setDataloaderSrc(gst::DataLoaderSrc src)
{
    DS_ASSERT(src.gstElement);
    add(src.gstElement);
    _dataloaderSrc = std::move(src);
}

/**
 * @brief configure action for appsink (type: ds3d::datarender from yaml)
 * @param sink pass ownership of the gstreamer element to the application layer (from header instantiation)
 */
void DepthCameraApp::setDataRenderSink(gst::DataRenderSink sink)
{
    DS_ASSERT(sink.gstElement);
    add(sink.gstElement);
    _datarenderSink = std::move(sink);
}

/**
 * @brief stop pipeline and action objects for their elements
 * @return ErrCode for flow handling
 */
ErrCode DepthCameraApp::stop()
{
    if (_dataloaderSrc.customProcessor)
    {
        _dataloaderSrc.customProcessor.stop();
        _dataloaderSrc.gstElement.reset();
        _dataloaderSrc.customProcessor.reset();
    }

    if (_datarenderSink.customProcessor)
    {
        _datarenderSink.customProcessor.stop();
        _datarenderSink.gstElement.reset();
        _datarenderSink.customProcessor.reset();
    }
    ErrCode c = ds3d::app::Ds3dAppContext::stop();
    return c;
}

/**
 * @brief stop and clear all object memory
 *
 */
void DepthCameraApp::deinit()
{
    ds3d::app::Ds3dAppContext::deinit();
    _datarenderSink.customlib.reset();
    _dataloaderSrc.customlib.reset();
}


/**
 * @brief callback function to read Gstreamer's bus messages from application and elements
 * @param msg the message passed to the bus from the gstreamer application layer
 * @return bool : true if successful (mandatory return for bus_call); otherwise critical error and stops application
 */
bool DepthCameraApp::busCall(GstMessage *msg)
{
    DS_ASSERT(mainLoop());
    switch (GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            LOG_INFO("End of stream\n");
            quitMainLoop();
            break;
        case GST_MESSAGE_ERROR:
        {
            gchar *debug = nullptr;
            GError *error = nullptr;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("ERROR from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
            if (debug)
                g_printerr("Error details: %s\n", debug);
            g_free(debug);
            g_error_free(error);

            quitMainLoop();
            break;
        }
        default:
            break;
    }
    return TRUE;
}