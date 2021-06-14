#pragma once

#include "Qv2rayBaseLibrary.hpp"

namespace Qv2rayBase
{
    class Qv2rayBaseLibraryPrivate
    {
      public:
        Qv2rayBase::Models::Qv2rayBaseConfigObject *configuration;

        Plugin::LatencyTestHost *latencyTestHost;
        Plugin::PluginAPIHost *pluginAPIHost;
        Plugin::PluginManagerCore *pluginCore;
        Profile::ProfileManager *profileManager;
        Profile::KernelManager *kernelManager;
        QFlags<Qv2rayStartFlags> startupFlags;

        Interfaces::IConfigurationGenerator *configGenerator;
        Interfaces::IStorageProvider *storageProvider;
    };
} // namespace Qv2rayBase
