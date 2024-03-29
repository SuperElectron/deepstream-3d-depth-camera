#ifndef _DS3D_COMMON_HPP_DATALOADER_HPP
#define _DS3D_COMMON_HPP_DATALOADER_HPP

#include "3d/common/common.h"
#include "3d/common/func_utils.h"
#include "dataprocess.hpp"

/**
 * @file GuardDataLoader is the safe access entry for abiDataLoader (appsrc)
 */

namespace ds3d {

/**
 * @brief GuardDataLoader is the safe access entry for abiDataLoader.
 *   Applications can use it to make C-based APIs safer. it would manage
 *   abiRefDataLoader automatically. with that, App user do not need to
 *   refCopy_i or destroy abiRefDataLoader manually.
 *
 *   For example:
 *     abiRefDataLoader* rawRefLoader = creator();
 *     GuardDataLoader guardLoader(rawRefLoader, true); // take the ownership of rawRefLoader
 *     guardLoader.setUserData(userdata, [](void*){ ...free... });
 *     guardLoader.setErrorCallback([](ErrCode c, const char* msg){ stderr << msg; });
 *     ErrCode c = guardLoader.start(config, path);
 *     DS_ASSERT(isGood(c));
 *     c = guardLoader.start(config, path);  // invoke abiDataLoader::start_i(...)
 *     GuardDataMap data;
 *     c = guardLoader.read(data); // invoke abiDataLoader::read_i(...)
 *     DS_ASSERT(isGood(c));
 *     DS_ASSERT(data);
 *     ... // access data
 *     c = guardLoader.stop(); // invoke abiDataLoader::stop_i(...)
 *     guardLoader.reset(); // destroy abiRefDataLoader, when all reference
 *                          // destroyed, abiDataLoader would be freed.
 */
class GuardDataLoader : public GuardDataProcess<abiDataLoader> {
    using _Base = GuardDataProcess<abiDataLoader>;

public:
    // GuardDataLoader() = default;
    template <typename... Args /*, _EnableIfConstructible<_Base, Args...> = true*/>
    GuardDataLoader(Args&&... args) : _Base(std::forward<Args>(args)...)
    {
    }
    ~GuardDataLoader() = default;

    std::string getOutputCaps()
    {
        DS_ASSERT(ptr());
        return cppString(ptr()->getCaps_i(CapsPort::kOutput));
    }

    ErrCode readData(GuardDataMap& datamap)
    {
        abiRefDataMap* data = nullptr;
        DS_ASSERT(ptr());
        ErrCode code = ptr()->readData_i(data);
        datamap.reset(data);
        return code;
    }

    ErrCode readDataAsync(abiOnDataCB::CppFunc dataReadyCB)
    {
        DS_ASSERT(ptr());
        DS_ASSERT(dataReadyCB);
        GuardCB<abiOnDataCB> guardCb;
        guardCb.setFn<ErrCode, const abiRefDataMap*>(std::move(dataReadyCB));
        return ptr()->readDataAsync_i(guardCb.abiRef());
    }
};

}  // namespace ds3d

#endif  // _DS3D_COMMON_HPP_DATALOADER_HPP