#pragma once
#include "QvPluginInterface.hpp"

namespace Qv2rayBase::Profile
{
    class KernelManagerPrivate
    {
      public:
        const static inline QString QV2RAYBASE_DEFAULT_KERNEL_PLACEHOLDER = "__default__";
        QMap<QString, PluginIOBoundData> inboundInfo;
        QMap<QString, PluginIOBoundData> outboundInfo;
        qsizetype logPadding = 0;
        ConnectionGroupPair current;
        std::list<std::pair<QString, std::unique_ptr<Qv2rayPlugin::PluginKernel>>> kernels;
    };
} // namespace Qv2rayBase::Profile
