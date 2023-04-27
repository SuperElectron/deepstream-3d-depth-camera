//#include "application.h"
//
//using namespace ds3d;
//
///** Set global gAppCtx */
//static std::weak_ptr <DepthCameraApp> gAppCtx;
//
//void static WindowClosed() {
//    LOG_DEBUG("Window closed.");
//    auto appCtx = gAppCtx.lock();
//
//    if (!appCtx)
//    {
//        return;
//    }
//
//    if (appCtx->isRunning(5000))
//    {
//        appCtx->sendEOS();
//    } else
//    {
//        appCtx->quitMainLoop();
//    }
//}
//
//static void help(const char *bin)
//{
//    printf("Usage: %s -c <deepstream-3d-depth-camera-config.yaml>\n", bin);
//    printf("Usage: %s -c /src/configs/example_render.yaml \n", bin);
//}
//
///**
// * Function to handle program interrupt signal.
// * It installs default handler after handling the interrupt.
// */
//static void _intr_handler(int signum)
//{
//    LOG_INFO("User Interrupted..");
//
//    ShrdPtr <DepthCameraApp> appCtx = gAppCtx.lock();
//    if (appCtx)
//    {
//        if (appCtx->isRunning())
//        {
//            appCtx->sendEOS();
//        } else
//        {
//            appCtx->quitMainLoop();
//        }
//    } else
//    {
//        LOG_ERROR("program terminated.");
//        std::terminate();
//    }
//}
//
//
///**
// * Function to install custom handler for program interrupt signal.
// */
//static void _intr_setup(void)
//{
//    struct sigaction action;
//    memset(&action, 0, sizeof(action));
//    action.sa_handler = _intr_handler;
//    sigaction(SIGINT, &action, NULL);
//}
//
//#undef CHECK_ERROR
//#define CHECK_ERROR(statement, fmt, ...) DS3D_FAILED_RETURN(statement, -1, fmt, ##__VA_ARGS__)
//
//#undef RETURN_ERROR
//#define RETURN_ERROR(statement, fmt, ...) DS3D_ERROR_RETURN(statement, fmt, ##__VA_ARGS__)
//
//using ConfigList = std::vector<config::ComponentConfig>;
//
///**
// * Function to create dataloader source.
// */
//static ErrCode CreateLoaderSource(
//        std::map <config::ComponentType,
//        ConfigList> &configTable,
//        gst::DataLoaderSrc &loaderSrc,
//        bool startLoader)
//        {
//    // Check whether dataloader is configured
//    DS3D_FAILED_RETURN(
//            configTable.count(config::ComponentType::kDataLoader),
//            ErrCode::kConfig,
//            "config file doesn't have dataloader types"
//    );
//    DS_ASSERT(configTable[config::ComponentType::kDataLoader].size() == 1);
//    config::ComponentConfig &srcConfig = configTable[config::ComponentType::kDataLoader][0];
//
//    // creat appsrc and dataloader
//    DS3D_ERROR_RETURN(
//            NvDs3D_CreateDataLoaderSrc(srcConfig, loaderSrc, startLoader),
//            "Create appsrc and dataloader failed"
//    );
//    DS_ASSERT(loaderSrc.gstElement);
//    DS_ASSERT(loaderSrc.customProcessor);
//
//    return ErrCode::kGood;
//}
//
///**
// * Function to create datarender sink.
// */
//static ErrCode CreateRenderSink(
//        std::map <config::ComponentType,
//        ConfigList> &configTable,
//        gst::DataRenderSink &renderSink,
//        bool startRender)
//    {
//
//    // Check whether datarender is configured
//    if (configTable.find(config::ComponentType::kDataRender) == configTable.end())
//    {
//        LOG_INFO("config file does not have datarender component, using fakesink instead");
//        renderSink.gstElement = gst::elementMake("fakesink", "fakesink");
//        DS_ASSERT(renderSink.gstElement);
//        return ErrCode::kGood;
//    }
//
//    DS3D_FAILED_RETURN(
//            configTable[config::ComponentType::kDataRender].size() == 1,
//            ErrCode::kConfig,
//            "multiple datarender component found, please update and keep 1 render only"
//    );
//
//    config::ComponentConfig &sinkConfig = configTable[config::ComponentType::kDataRender][0];
//
//    // creat appsink and datarender
//    DS3D_ERROR_RETURN(
//            NvDs3D_CreateDataRenderSink(sinkConfig, renderSink, startRender),
//            "Create appsink and datarender failed"
//    );
//    DS_ASSERT(renderSink.gstElement);
//    DS_ASSERT(renderSink.customProcessor);
//
//    return ErrCode::kGood;
//}
