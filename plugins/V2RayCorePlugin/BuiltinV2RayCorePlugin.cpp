#include "BuiltinV2RayCorePlugin.hpp"

namespace Qv2rayBase::BuiltinPlugins::V2RayCore
{
    const QvPluginMetadata BuiltinV2RayCorePlugin::GetMetadata() const
    {
        return { "Qv2rayBase Builtin V2Ray Core Plugin",                             //
                 "Qv2rayBase Development Team",                                      //
                 "qvplugin_builtin_v2ray_core",                                      //
                 "Builtin Latency Test Engine. Provides basic V2Ray kernel support", //
                 "",                                                                 //
                 { COMPONENT_KERNEL, COMPONENT_OUTBOUND_HANDLER } };
    }

    BuiltinV2RayCorePlugin::~BuiltinV2RayCorePlugin()
    {
    }

    bool BuiltinV2RayCorePlugin::InitializePlugin()
    {
        return true;
    }

    void BuiltinV2RayCorePlugin::SettingsUpdated()
    {
    }

} // namespace Qv2rayBase::BuiltinPlugins::V2RayCore
