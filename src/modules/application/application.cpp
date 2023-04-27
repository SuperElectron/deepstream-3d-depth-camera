#include "application.h"

using namespace myApp;


Application::Application(){};

Application::~Application(){};


/**
 * @brief display usage message when user tries to run the program wrong
 * @param path location of callable executable
 */
void Application::help(const char *path)
{
    printf("Usage: %s -c <deepstream-3d-depth-camera-config.yaml>\n", path);
    printf("Usage: %s -c /src/configs/example_render.yaml \n", path);
};


/**
 * @brief Function to create dataloader source.
 *
 * @param configTable
 * @param loaderSrc
 * @param startLoader
 * @return ErrCode for flow handling
 */
ErrCode Application::CreateLoaderSource(
        std::map <config::ComponentType,
        ConfigList> &configTable,
        gst::DataLoaderSrc &loaderSrc,
        bool startLoader)
{
    // Check whether dataloader is configured
    DS3D_FAILED_RETURN(
            configTable.count(config::ComponentType::kDataLoader),
            ErrCode::kConfig,
            "config file doesn't have dataloader types"
    );
    DS_ASSERT(configTable[config::ComponentType::kDataLoader].size() == 1);
    config::ComponentConfig &srcConfig = configTable[config::ComponentType::kDataLoader][0];

    // creat appsrc and dataloader
    DS3D_ERROR_RETURN(
            NvDs3D_CreateDataLoaderSrc(srcConfig, loaderSrc, startLoader),
            "Create appsrc and dataloader failed"
    );
    DS_ASSERT(loaderSrc.gstElement);
    DS_ASSERT(loaderSrc.customProcessor);

    return ErrCode::kGood;
};

/**
 * @brief Function to create datarender sink.
 * @param configTable
 * @param renderSink
 * @param startRender
 * @return for flow handling
 */
ErrCode Application::CreateRenderSink(
        std::map <config::ComponentType,
        ConfigList> &configTable,
        gst::DataRenderSink &renderSink,
        bool startRender)
{

    // Check whether datarender is configured
    if (configTable.find(config::ComponentType::kDataRender) == configTable.end())
    {
        LOG_INFO("config file does not have datarender component, using fakesink instead");
        renderSink.gstElement = gst::elementMake("fakesink", "fakesink");
        DS_ASSERT(renderSink.gstElement);
        return ErrCode::kGood;
    }

    DS3D_FAILED_RETURN(
            configTable[config::ComponentType::kDataRender].size() == 1,
            ErrCode::kConfig,
            "multiple datarender component found, please update and keep 1 render only"
    );

    config::ComponentConfig &sinkConfig = configTable[config::ComponentType::kDataRender][0];

    // creat appsink and datarender
    DS3D_ERROR_RETURN(
            NvDs3D_CreateDataRenderSink(sinkConfig, renderSink, startRender),
            "Create appsink and datarender failed"
    );
    DS_ASSERT(renderSink.gstElement);
    DS_ASSERT(renderSink.customProcessor);

    return ErrCode::kGood;
};
