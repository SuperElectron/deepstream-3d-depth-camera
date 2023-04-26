#ifndef DS3D_APP_DEEPSTREAM_3D_CONTEXT_APP_H
#define DS3D_APP_DEEPSTREAM_3D_CONTEXT_APP_H

#include "gstnvdsmeta.h"

// inlcude all 3d hpp header files
#include <3d/common/hpp/dataloader.hpp>
#include <3d/common/hpp/datamap.hpp>
#include <3d/common/hpp/frame.hpp>
#include <3d/common/hpp/yaml_config.hpp>

// inlucde nvds3d Gst header files
#include <3d/gst/nvds3d_gst_plugin.h>
#include <3d/gst/nvds3d_gst_ptr.h>
#include <3d/gst/nvds3d_meta.h>
#include <gst/gst.h>

namespace ds3d {
    namespace app {

        class Ds3dAppContext {
        public:
            Ds3dAppContext() {}

            virtual ~Ds3dAppContext() { deinit(); }

            void setMainloop(GMainLoop *loop) { _mainLoop.reset(loop); }

            /*
             * @brief Set up the gstreamer pipeline, bus, and bus_watch
             *
             * @param name               @description string to name the pipeline
             * @returns     ErrCode      @description success
             */
            ErrCode init(const std::string &name) {
                // asset main loop exists (which runs pipeline) and pipeline is not yet created
                DS_ASSERT(_mainLoop);
                DS_ASSERT(!_pipeline);

                // create gstreamer pipeline
                _pipeline.reset(gst_pipeline_new(name.c_str()));
                DS3D_FAILED_RETURN(pipeline(), ErrCode::kGst, "create pipeline: %s failed", name.c_str());
                _pipeline.setName(name);

                // set up the pipeline's bus and set a watch, so we can catch messages (like EOS=end-of-stream)
                _bus.reset(gst_pipeline_get_bus(pipeline()));
                DS3D_FAILED_RETURN(bus(), ErrCode::kGst, "get bus from pipeline: %s failed", name.c_str());
                _busWatchId = gst_bus_add_watch(bus(), sBusCall, this);
                return ErrCode::kGood;
            }

            /*
             * @brief Adds elements to gst_pipeline
             *
             * @param ele               @description pointer to gstreamer element
             * @returns     string      @description pointer to class
             */
            Ds3dAppContext &add(const gst::ElePtr &ele) {
                DS_ASSERT(_pipeline);
                DS3D_THROW_ERROR(
                        gst_bin_add(GST_BIN(pipeline()), ele.copy()), ErrCode::kGst, "add element failed");
                _elementList.emplace_back(ele);
                return *this;
            }

            /*
             * @brief Set the pipeline STATE to playing which starts data flow
             */
            ErrCode play() {
                DS_ASSERT(_pipeline);
                return setPipelineState(GST_STATE_PLAYING);
            }

            /*
             * @brief Set the pipeline STATE to NULL which stops all data flow
             *
             */
            virtual ErrCode stop() {
                DS_ASSERT(_pipeline);
                ErrCode c = setPipelineState(GST_STATE_NULL);
                if (!isGood(c)) {
                    LOG_WARNING("set pipeline state to GST_STATE_NULL failed");
                }
                if (!isGood(c)) {
                    LOG_WARNING("set pipeline state to GST_STATE_NULL failed");
                }
                GstState end = GST_STATE_NULL;
                c = getState(_pipeline.get(), &end, nullptr, 3000);
                if (!isGood(c) || end != GST_STATE_NULL) {
                    LOG_WARNING("waiting for pipeline state to null failed, force to quit");
                }
                for (auto &each: _elementList) {
                    if (each) {
                        c = setState(each.get(), GST_STATE_NULL);
                    }
                }
                return c;
            }

            /*
             * @brief validate that the gstreamer pipeline is running
             *
             * @param       timeout         @description milliseconds, 0 means never timeout
             * @returns     boolean         @description true if it is running
             */
            bool isRunning(size_t timeout = 0) {
                DS_ASSERT(pipeline());

                // instantiate state variables to pause the pipeline for the duration of time requested
                GstState state = GST_STATE_NULL;
                GstState pending = GST_STATE_NULL;

                // get the pipeline state and wait for it to return (based on timeout)
                if (gst_element_get_state(
                        GST_ELEMENT(pipeline()), &state, &pending,
                        (timeout ? timeout * 1000000 : GST_CLOCK_TIME_NONE)) == GST_STATE_CHANGE_FAILURE) {
                    return false;
                }
                // return true if the pipeline state is PLAYING, or it is pending a change to PLAYING
                if (state == GST_STATE_PLAYING || pending == GST_STATE_PLAYING) {
                    return true;
                }
                // return false for any other result of gst_element_get_state(...)
                return false;
            }

            /*
             * @brief terminate the gst_pipeline
             */
            void quitMainLoop() {
                if (mainLoop()) {
                    g_main_loop_quit(mainLoop());
                }
            }

            /*
             * @brief start the main thread that runs the gstreamer pipeline
             */
            void runMainLoop() {
                if (mainLoop()) {
                    g_main_loop_run(mainLoop());
                }
            }

            /*
             * @brief clear bus, pipeline, and all the element pointers that were added to the pipeline
             */
            virtual void deinit() {
                if (bus()) {
                    gst_bus_remove_watch(bus());
                }
                _bus.reset();
                _pipeline.reset();
                _elementList.clear();
                _mainLoop.reset();
            }

            /*
             * @brief send EOS into pipeline (tells all elements to terminate once they have no more data)
             */
            ErrCode sendEOS() {
                DS3D_FAILED_RETURN(
                        gst_element_send_event(GST_ELEMENT(pipeline()), gst_event_new_eos()), ErrCode::kGst,
                        "send EOS failed");
                return ErrCode::kGood;
            }

            GstPipeline *pipeline() const { return GST_PIPELINE_CAST(_pipeline.get()); }

            GstBus *bus() const { return _bus.get(); }

            GMainLoop *mainLoop() const { return _mainLoop.get(); }

        private:
            // no need to free msg
            virtual bool busCall(GstMessage *msg) = 0;

        protected:

            /*
             * @brief sets the pipeline state GST_STATE_<NULL, READY, PLAYING, PAUSED>
             *
             * @param       state           @description desired GST_STATE_<> from above
             * @returns     ErrCode         @description ErrCode::kGood if successful
             */
            ErrCode setPipelineState(GstState state) {
                DS_ASSERT(_pipeline);
                return setState(_pipeline.get(), state);
            }

            /*
             * @brief validate that the gstreamer pipeline is running
             *
             * @param       ele             @description set the state on an element (usually gst_pipeline)
             * @param       state           @description desired GST_STATE_<>
             * @returns     boolean         @description true if it is running
             */
            ErrCode setState(GstElement *ele, GstState state) {
                DS_ASSERT(ele);
                DS3D_FAILED_RETURN(
                        gst_element_set_state(ele, state) != GST_STATE_CHANGE_FAILURE, ErrCode::kGst,
                        "element set state: %d failed", state);
                return ErrCode::kGood;
            }

            /*
             * @brief get element states
             *
             * @param       timeout         @description timeout in milliseconds to wait for state retrieval
             * @returns     boolean         @description ErrCode that describes success or failure
             */
            ErrCode getState(
                    GstElement *ele, GstState *state, GstState *pending = nullptr, size_t timeout = 0) {
                DS_ASSERT(ele);
                GstStateChangeReturn ret = gst_element_get_state(
                        ele, state, pending, (timeout ? timeout * 1000000 : GST_CLOCK_TIME_NONE));
                switch (ret) {
                    case GST_STATE_CHANGE_FAILURE:
                        return ErrCode::kGst;
                    case GST_STATE_CHANGE_SUCCESS:
                    case GST_STATE_CHANGE_NO_PREROLL:
                        return ErrCode::kGood;
                    default:
                        return ErrCode::kUnknown;
                }
                return ErrCode::kGood;
            }

            /*
             * @brief passes message from the pipeline's bus_call to application context (ctx).
             *  This is the set on gst_bus_add_watch() to act as a callback when messages from elements are passed to
             *  the internal gstreamer application bus.
             *
             * @param       bus             @description the bus (member of the gst_element pipeline)
             * @param       msg             @description the message that was caught by the bus callback function
             * @param       data            @description custom data passed into the CB function (pointer to self)
             * @returns     boolean         @description true if it is running
             */
            static gboolean sBusCall(GstBus *bus, GstMessage *msg, gpointer data) {
                Ds3dAppContext *ctx = static_cast<Ds3dAppContext *>(data);
                DS_ASSERT(ctx->bus() == bus);
                return ctx->busCall(msg);
            }

            // members
            gst::ElePtr _pipeline;
            gst::BusPtr _bus;
            uint32_t _busWatchId = 0;
            std::vector <gst::ElePtr> _elementList;
            ds3d::UniqPtr<GMainLoop> _mainLoop{nullptr, g_main_loop_unref};
            DS3D_DISABLE_CLASS_COPY(Ds3dAppContext);
        };

    }
}  // namespace 3d::app

#endif  // DS3D_APP_DEEPSTREAM_3D_CONTEXT_APP_H
