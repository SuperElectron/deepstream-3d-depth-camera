#ifndef DS3D_COMMON_HPP_CONFIG_HPP
#define DS3D_COMMON_HPP_CONFIG_HPP

#include "3d/common/common.h"
#include "3d/common/idatatype.h"
#include "3d/common/type_trait.h"

/**
 * @file maps yaml config types to their appropriate elements (appsrc, nvds3dfilter, appsink)
 */

namespace ds3d {
namespace config {

enum class ComponentType : int {
    kNone = 0,
    kDataLoader = 1,
    kDataFilter = 2,
    kDataRender = 3,
    kUserApp = 4,
};

struct ComponentConfig {
    std::string name;
    ComponentType type = ComponentType::kNone;
    std::string gstInCaps;
    std::string gstOutCaps;
    std::string customLibPath;
    std::string customCreateFunction;
    std::string configBody;

    // raw data
    std::string rawContent;
    std::string filePath;
};

inline ComponentType
componentType(const std::string& strType)
{
    const static std::unordered_map<std::string, ComponentType> kTypeMap = {
        {"ds3d::dataloader", ComponentType::kDataLoader},
        {"ds3d::datafilter", ComponentType::kDataFilter},
        {"ds3d::datarender", ComponentType::kDataRender},
        {"ds3d::userapp", ComponentType::kUserApp},
    };
    DS_ASSERT(!strType.empty());
    auto tpIt = kTypeMap.find(strType);
    DS3D_FAILED_RETURN(
        tpIt != kTypeMap.end(), ComponentType::kNone, "component type: %s is not supportted.",
        strType.c_str());
    return tpIt->second;
}

inline const char*
componentTypeStr(ComponentType type)
{
    switch(type) {
    case ComponentType::kDataLoader: return "ds3d::dataloader";
    case ComponentType::kDataFilter: return "ds3d::datafilter";
    case ComponentType::kDataRender: return "ds3d::datarender";
    case ComponentType::kUserApp:
        return "ds3d::userapp";
    default: return "ds3d::unknown::component";
    }
}

}}  // namespace ds3d::config

#endif  // DS3D_COMMON_HPP_CONFIG_HPP