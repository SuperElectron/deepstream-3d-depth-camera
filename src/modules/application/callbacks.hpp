#pragma once
#include "application.h"

using namespace ds3d;

/**
 * @brief callback on appsrc (yaml config type: ds3d::dataloader) to process queued data on the pad
 *
 * @param pad the pad on which the callback was placed on
 * @param info data passed into the callback  (a wrapped GST_BUFFER for GstPadProbeCallback)
 * @param udata custom user defined data (unused)
 * @return GstPadProbeReturn to manage data and signals sent downstream into the pipeline (push data, fail signal, ...)
 *  @ref: (https://gstreamer.freedesktop.org/documentation/gstreamer/gstpad.html?gi-language=c#GstPadProbeReturn)
 */
static GstPadProbeReturn appsrcBufferProbe(GstPad *pad, GstPadProbeInfo *info, gpointer udata)
{
    DepthCameraApp *appCtx = (DepthCameraApp *) udata;
    GstBuffer *buf = (GstBuffer *) info->data;
    ErrCode c = ErrCode::kGood;

    DS3D_UNUSED(c);
    DS_ASSERT(appCtx);
    AppProfiler &profiler = appCtx->profiler();

    if (!NvDs3D_IsDs3DBuf(buf)) {
        LOG_WARNING("appsrc buffer is not DS3D buffer");
    }
    const abiRefDataMap *refDataMap = nullptr;
    if (!isGood(NvDs3D_Find1stDataMap(buf, refDataMap)))
    {
        LOG_ERROR("didn't find datamap from GstBuffer, need to stop");
        if (appCtx->isRunning(5000))
        {
            appCtx->sendEOS();
        }
        return GST_PAD_PROBE_DROP;
    }

    DS_ASSERT(refDataMap);
    GuardDataMap dataMap(*refDataMap);
    DS_ASSERT(dataMap);
    Frame2DGuard depthFrame;
    if (dataMap.hasData(kDepthFrame))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getGuardData (kDepthFrame, depthFrame)),
                GST_PAD_PROBE_DROP,
                "get depthFrame failed"
        );
        DS_ASSERT(depthFrame);
        DS_ASSERT(depthFrame->dataType() == DataType::kUint16);
        Frame2DPlane p = depthFrame->getPlane(0);
        DepthScale scale;
        c = dataMap.getData(kDepthScaleUnit, scale);
        DS_ASSERT(isGood(c));
        LOG_DEBUG("depth frame is found, depth-scale: %.04f, w: %u, h: %u", scale.scaleUnit, p.width, p.height);
    }

    Frame2DGuard colorFrame;
    if (dataMap.hasData(kColorFrame))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getGuardData (kColorFrame, colorFrame)),
                GST_PAD_PROBE_DROP,
                "get color Frame failed"
        );
        DS_ASSERT(colorFrame);
        DS_ASSERT(colorFrame->dataType() == DataType::kUint8);
        Frame2DPlane p = colorFrame->getPlane(0);
        LOG_DEBUG("RGBA frame is found,  w: %d, h: %d", p.width, p.height);
    }

    // dump depth data for debug
    if (depthFrame && profiler.depthWriter.isOpen())
    {
        DS_ASSERT(depthFrame->memType() != MemType::kGpuCuda);
        DS3D_FAILED_RETURN(
                profiler.depthWriter.write ((const uint8_t *) depthFrame->base(), depthFrame->bytes()),
                GST_PAD_PROBE_DROP, "Dump depth data failed");
    }

    // dump color data for debug
    if (colorFrame && profiler.colorWriter.isOpen())
    {
        DS_ASSERT(colorFrame->memType() != MemType::kGpuCuda);
        DS3D_FAILED_RETURN(
                profiler.colorWriter.write ((const uint8_t *) colorFrame->base(), colorFrame->bytes()),
                GST_PAD_PROBE_DROP, "Dump color data failed");
    }

    return GST_PAD_PROBE_OK;
}

/**
 * @brief callback on appsink (yaml config type: ds3d::datarender) to process queued data on the pad
 *
 * @param pad the pad on which the callback was placed on
 * @param info data passed into the callback  (a wrapped GST_BUFFER for GstPadProbeCallback)
 * @param udata custom user defined data (unused)
 * @return GstPadProbeReturn to manage data and signals sent downstream into the pipeline (push data, fail signal, ...)
 *  @ref: (https://gstreamer.freedesktop.org/documentation/gstreamer/gstpad.html?gi-language=c#GstPadProbeReturn)
 */
static GstPadProbeReturn appsinkBufferProbe(GstPad *pad, GstPadProbeInfo *info, gpointer udata)
{
    DepthCameraApp *appCtx = (DepthCameraApp *) udata;
    GstBuffer *buf = (GstBuffer *) info->data;

    DS_ASSERT(appCtx);
    AppProfiler &profiler = appCtx->profiler();

    if (!NvDs3D_IsDs3DBuf(buf))
    {
        LOG_WARNING("appsink buffer is not DS3D buffer");
    }
    const abiRefDataMap *refDataMap = nullptr;
    if (!isGood(NvDs3D_Find1stDataMap(buf, refDataMap)))
    {
        LOG_ERROR("didn't find datamap from GstBuffer, need to stop");
        if (appCtx->isRunning(5000))
        {
            appCtx->sendEOS();
        }
        return GST_PAD_PROBE_DROP;
    }

    uint32_t numPoints = 0;
    DS_ASSERT(refDataMap);
    GuardDataMap dataMap(*refDataMap);
    DS_ASSERT(dataMap);

    FrameGuard pointFrame;
    if (dataMap.hasData(kPointXYZ))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getGuardData (kPointXYZ, pointFrame)),
                GST_PAD_PROBE_DROP,
                "get pointXYZ frame failed."
        );
        DS_ASSERT(pointFrame);
        DS_ASSERT(pointFrame->dataType() == DataType::kFp32);
        DS_ASSERT(pointFrame->frameType() == FrameType::kPointXYZ);
        Shape pShape = pointFrame->shape();  // N x 3
        DS_ASSERT(pShape.numDims == 2 && pShape.d[1] == 3);  // PointXYZ
        numPoints = (size_t) pShape.d[0];
        LOG_DEBUG("pointcloudXYZ frame is found, points num: %u", numPoints);
    }

    FrameGuard colorCoord;
    if (dataMap.hasData(kPointCoordUV))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getGuardData (kPointCoordUV, colorCoord)),
                GST_PAD_PROBE_DROP,
                "get PointCoordUV frame failed."
        );
        DS_ASSERT(colorCoord);
        DS_ASSERT(colorCoord->dataType() == DataType::kFp32);
        Shape cShape = colorCoord->shape();  // N x 2
        DS_ASSERT(cShape.numDims == 2 && cShape.d[1] == 2);  // PointColorCoord
        numPoints = (size_t) cShape.d[0];
        LOG_DEBUG("PointColorCoord frame is found,  points num: %u", numPoints);
    }

    // get depth & color intrinsic parameters, and also get depth-to-color extrinsic parameters
    IntrinsicsParam depthIntrinsics;
    IntrinsicsParam colorIntrinsics;
    ExtrinsicsParam d2cExtrinsics;  // rotation matrix is in the column-major order
    if (dataMap.hasData(kDepthIntrinsics))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getData (kDepthIntrinsics, depthIntrinsics)),
                GST_PAD_PROBE_DROP,
                "get depth intrinsic parameters failed."
        );
        LOG_DEBUG("DepthIntrinsics parameters is found, fx: %.4f, fy: %.4f", depthIntrinsics.fx, depthIntrinsics.fy);
    }
    if (dataMap.hasData(kColorIntrinsics))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getData (kColorIntrinsics, colorIntrinsics)),
                GST_PAD_PROBE_DROP,
                "get color intrinsic parameters failed."
        );
        LOG_DEBUG("ColorIntrinsics parameters is found, fx: %.4f, fy: %.4f", colorIntrinsics.fx, colorIntrinsics.fy);
    }
    if (dataMap.hasData(kDepth2ColorExtrinsics))
    {
        DS3D_FAILED_RETURN(
                isGood(dataMap.getData (kDepth2ColorExtrinsics, d2cExtrinsics)),
                GST_PAD_PROBE_DROP,
                "get depth2color extrinsic parameters failed."
        );
        LOG_DEBUG("depth2color extrinsic parameters is found, t:[%.3f, %.3f, %3.f]",
                  d2cExtrinsics.translation.x, d2cExtrinsics.translation.y, d2cExtrinsics.translation.z
        );
    }

    // dump depth data for debug
    if (pointFrame && profiler.pointWriter.isOpen()) {
        DS_ASSERT(pointFrame->memType() != MemType::kGpuCuda);
        DS3D_FAILED_RETURN(
                profiler.pointWriter.write((const uint8_t *) pointFrame->base(), pointFrame->bytes()),
                GST_PAD_PROBE_DROP, "Dump points data failed"
        );
    }

    return GST_PAD_PROBE_OK;
}
