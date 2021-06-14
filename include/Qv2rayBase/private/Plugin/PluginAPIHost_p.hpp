#pragma once
#include "QvPluginInterface.hpp"

namespace Qv2rayBase::Plugin
{
    class PluginAPIHostPrivate
    {
      public:
        QHash<KernelId, Qv2rayPlugin::KernelFactory> kernels;
        QHash<LatencyTestEngineId, Qv2rayPlugin::LatencyTestEngineInfo> latencyTesters;
    };
} // namespace Qv2rayBase::Plugin
