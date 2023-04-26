#ifndef DS3D_COMMON_HPP_DATA_RENDER_HPP
#define DS3D_COMMON_HPP_DATA_RENDER_HPP

#include "3d/common/common.h"
#include "3d/common/func_utils.h"
#include "dataprocess.hpp"

/**
 * @file GuardDataRender is the safe access entry for abiDataRender (appsink)
 */

namespace ds3d {

using GuardWindow = GuardDataT<abiWindow>;

/**
 * @brief GuardDataRender is the safe access entry for abiDataRender.
 *   Applications can use it to make C-based APIs safer. it would manage
 *   abiRefDataRender automatically. with that, App user do not need to
 *   refCopy_i or destroy abiRefDataRender manually.
 *
 *   For example:
 *     abiRefDataRender* rawRefRender = createRender();
 *     GuardDataRender guardRender(rawRefRender, true); // take the ownership of rawRefRender
 *     guardRender.setUserData(userdata, [](void*){ ...free... });
 *     guardRender.setErrorCallback([](ErrCode c, const char* msg){ stderr << msg; });
 *     ErrCode c = guardRender.start(config, path);
 *     DS_ASSERT(isGood(c));
 *     c = guardRender.start(config, path);  // invoke abiDataRender::start_i(...)
 *     GuardDataMap data = ...;
 *     c = guardRender.preroll(data); // invoke abiDataRender::preroll_i(...)
 *     DS_ASSERT(isGood(c));
 *     // invoke abiDataRender::render_i(...)
 *     c = guardRender.render(data,
 *         [](ErrCode c, const abiRefDataMap* d){
 *            GuardDataMap doneData(*d); // check ErrCode and data.
 *         });
 *     DS_ASSERT(isGood(c));
 *     c = guardRender.flush(); // flush all data in queue
 *     c = guardRender.stop(); // invoke abiDataRender::stop_i(...)
 *     guardRender.reset(); // destroy abiRefDataRender, when all reference
 *                          // destroyed, abiDataRender would be freed.
 */
class GuardDataRender : public GuardDataProcess<abiDataRender> {
    using _Base = GuardDataProcess<abiDataRender>;

public:
    template <typename... Args>
    GuardDataRender(Args&&... args) : _Base(std::forward<Args>(args)...)
    {
    }
    ~GuardDataRender() = default;

    ErrCode preroll(GuardDataMap datamap)
    {
        DS_ASSERT(ptr());
        return ptr()->preroll_i(datamap.abiRef());
    }

    ErrCode render(GuardDataMap datamap, abiOnDataCB::CppFunc dataDoneCB)
    {
        GuardCB<abiOnDataCB> cb;
        cb.setFn<ErrCode, const abiRefDataMap*>(std::move(dataDoneCB));
        DS_ASSERT(ptr());
        ErrCode code = ptr()->render_i(datamap.abiRef(), cb.abiRef());
        return code;
    }

    GuardWindow getWindow() const {
        DS_ASSERT(ptr());
        const abiRefWindow* refWin = ptr()->getWindow_i();
        if (refWin) {
            return GuardWindow(*refWin);
        } else {
            return nullptr;
        }
    }
};

}  // namespace ds3d

#endif  // DS3D_COMMON_HPP_DATA_RENDER_HPP