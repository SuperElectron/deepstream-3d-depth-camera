//#include <cuda_runtime_api.h>
#include <unistd.h>
#include "camera.hpp"

using namespace ds3d;


int main(int argc, char *argv[])
{
    gst::DataLoaderSrc loaderSrc;
    gst::DataRenderSink renderSink;
    std::string configPath;
    std::string configContent;

    /* Standard GStreamer initialization */
    gst_init(&argc, &argv);

    /* setup signal handler */
    _intr_setup();

    /* Parse program arguments */
    opterr = 0;
    int c = -1;
    while ((c = getopt(argc, argv, "hc:")) != -1) {
        switch (c) {
            case 'c':  // get config file path
                configPath = optarg;
                break;
            case 'h':
                help(argv[0]);
                return 0;
            case '?':
            default:
                help(argv[0]);
                return -1;
        }
    }
    // if no argument or incorrect arguments passed, display how to call the binary
    if (configPath.empty()) {
        LOG_ERROR("config file is not set!");
        help(argv[0]);
        return -1;
    }
    CHECK_ERROR(readFile(configPath, configContent), "read file: %s failed", configPath.c_str());

    // parse all components in config file
    ConfigList componentConfigs;
    ErrCode code =
            ds3d::config::CatchConfigCall(config::parseFullConfig, configContent, configPath, componentConfigs);
    CHECK_ERROR(isGood(code), "parse config failed");

    // Order all parsed component configs into config table
    std::map <config::ComponentType, ConfigList> configTable;
    for (const auto &c: componentConfigs) {
        configTable[c.type].emplace_back(c);
    }

    ShrdPtr <DepthCameraApp> appCtx = std::make_shared<DepthCameraApp>();
    gAppCtx = appCtx;

    // update userapp configuration
    if (configTable.count(config::ComponentType::kUserApp)) {
        CHECK_ERROR(
                isGood(appCtx->initUserAppProfiling(configTable[config::ComponentType::kUserApp][0])),
                "parse userapp data failed");
    }

    // Initialize app context with main loop and pipelines
    appCtx->setMainloop(g_main_loop_new(NULL, FALSE));
    CHECK_ERROR(appCtx->mainLoop(), "set main loop failed");
    CHECK_ERROR(isGood(appCtx->init("deepstream-depth-camera-pipeline")), "init pipeline failed");

    bool startLoaderDirectly = true;
    bool startRenderDirectly = true;
    CHECK_ERROR(
            isGood(CreateLoaderSource(configTable, loaderSrc, startLoaderDirectly)),
            "create dataloader source failed"
    );

    CHECK_ERROR(
            isGood(CreateRenderSink(configTable, renderSink, startRenderDirectly)),
            "create datarender sink failed"
    );

    appCtx->setDataloaderSrc(loaderSrc);
    appCtx->setDataRenderSink(renderSink);

    DS_ASSERT(loaderSrc.gstElement);
    DS_ASSERT(renderSink.gstElement);

    /* create and add all filters */
    bool hasFilters = configTable.count(config::ComponentType::kDataFilter);
    /* link all pad/elements together */
    code = CatchVoidCall([&loaderSrc, &renderSink, hasFilters, &configTable, appCtx]() {
        gst::ElePtr lastEle = loaderSrc.gstElement;
        if (hasFilters) {
            auto &filterConfigs = configTable[config::ComponentType::kDataFilter];
            DS_ASSERT(filterConfigs.size());
            for (size_t i = 0; i < filterConfigs.size(); ++i) {
                auto queue = gst::elementMake("queue", ("filterQueue" + std::to_string(i)).c_str());
                DS_ASSERT(queue);
                auto filter =
                        gst::elementMake("nvds3dfilter", ("filter" + std::to_string(i)).c_str());
                DS3D_THROW_ERROR_FMT(
                        filter, ErrCode::kGst, "gst-plugin: %s is not found", "nvds3dfilter");
                g_object_set(
                        G_OBJECT(filter.get()), "config-content", filterConfigs[i].rawContent.c_str(),
                        nullptr);
                appCtx->add(queue).add(filter);
                lastEle.link(queue).link(filter);
                lastEle = filter;
            }
        }
        lastEle.link(renderSink.gstElement);
    });
    CHECK_ERROR(isGood(code), "Link pipeline elements failed");

    /* Add probe to get informed of the meta data generated, we add probe to gstappsrc src pad of the dataloader */
    gst::PadPtr srcPad = loaderSrc.gstElement.staticPad("src");
    CHECK_ERROR(srcPad, "appsrc src pad is not detected.");
    srcPad.addProbe(GST_PAD_PROBE_TYPE_BUFFER, appsrcBufferProbe, appCtx.get(), NULL);
    srcPad.reset();

    /* Add probe to get informed of the meta data generated, we add probe to gstappsink sink pad of the datrender */
    if (renderSink.gstElement) {
        gst::PadPtr sinkPad = renderSink.gstElement.staticPad("sink");
        CHECK_ERROR(sinkPad, "appsink sink pad is not detected.");
        sinkPad.addProbe(GST_PAD_PROBE_TYPE_BUFFER, appsinkBufferProbe, appCtx.get(), NULL);
        sinkPad.reset();
    }

    CHECK_ERROR(isGood(appCtx->play()), "app context play failed");
    LOG_INFO("Play...");

    // get window system and set close event callback
    if (renderSink.customProcessor) {
        GuardWindow win = renderSink.customProcessor.getWindow();
        if (win) {
            GuardCB <abiWindow::CloseCB> windowClosedCb;
            windowClosedCb.setFn<>(WindowClosed);
            win->setCloseCallback(windowClosedCb.abiRef());
        }
    }

    /* Wait till pipeline encounters an error or EOS */
    appCtx->runMainLoop();

    loaderSrc.reset();
    renderSink.reset();
    appCtx->stop();
    appCtx->deinit();

    return 0;
}
