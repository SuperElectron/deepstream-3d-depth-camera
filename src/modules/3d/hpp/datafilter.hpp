#ifndef DS3D_COMMON_HPP_DATA_FILTER_HPP
#define DS3D_COMMON_HPP_DATA_FILTER_HPP

#include "3d/common/common.h"
#include "3d/common/func_utils.h"
#include "dataprocess.hpp"

/**
 * @file GuardDataFilter is the safe access entry for abiDataFilter (nvds3dfilter)
 */

namespace ds3d {

/**
 * @brief GuardDataFilter is the safe access entry for abiDataFilter.
 *   Applications can use it to make C-based APIs safer. it would manage
 *   abiRefDataFilter automatically. with that, App user do not need to
 *   refCopy_i or destroy abiRefDataFilter manually.
 *
 *   For example:
 *     abiRefDataFilter* rawRef = creator();
 *     GuardDataFilter guardFilter(rawRef, true); // take the ownership of rawRef
 *     guardFilter.setUserData(userdata, [](void*){ ...free... });
 *     guardFilter.setErrorCallback([](ErrCode c, const char* msg){ stderr << msg; });
 *     ErrCode c = guardFilter.start(config, path);
 *     DS_ASSERT(isGood(c));
 *     c = guardFilter.start(config, path);  // invoke abiDataFilter::start_i(...)
 *     GuardDataMap inputData = ...; // prepare input data
 *     // invoke abiDataFilter::process_i(...)
 *     c = guardFilter.process(inputData,
 *         [](ErrCode c, const abiRefDataMap* d){
 *            GuardDataMap outputData(*d); // output data processing
 *            std::cout << "output data processing starts" << std::endl;
 *         },
 *         [](ErrCode c, const abiRefDataMap* d){
 *            GuardDataMap doneData(*d);
 *            std::cout << "input data consumed" << std::endl;
 *         });
 *     DS_ASSERT(isGood(c));
 *     //... wait for all data processed before stop
 *     c = guardFilter.flush();
 *     c = guardFilter.stop(); // invoke abiDataFilter::stop_i(...)
 *     guardFilter.reset(); // destroy abiRefDataFilter, when all reference
 *                          // destroyed, abiDataFilter would be freed.
 */
class GuardDataFilter : public GuardDataProcess<abiDataFilter> {
    using _Base = GuardDataProcess<abiDataFilter>;

public:
    template <typename... Args>
    GuardDataFilter(Args&&... args) : _Base(std::forward<Args>(args)...)
    {
    }
    ~GuardDataFilter() = default;

    ErrCode process(
        GuardDataMap datamap, abiOnDataCB::CppFunc outputDataCB,
        abiOnDataCB::CppFunc inputConsumedCB)
    {
        GuardCB<abiOnDataCB> guardOutputCb;
        GuardCB<abiOnDataCB> guardConsumedCb;
        guardOutputCb.setFn<ErrCode, const abiRefDataMap*>(std::move(outputDataCB));
        guardConsumedCb.setFn<ErrCode, const abiRefDataMap*>(std::move(inputConsumedCB));

        DS_ASSERT(ptr());
        ErrCode code =
            ptr()->process_i(datamap.abiRef(), guardOutputCb.abiRef(), guardConsumedCb.abiRef());
        return code;
    }
};

}  // namespace ds3d

#endif  // DS3D_COMMON_HPP_DATA_FILTER_HPP