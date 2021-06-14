#pragma once

#include "QvPluginInterface.hpp"

#include <QHash>
#include <QPluginLoader>

namespace Qv2rayBase::Plugin
{
    struct PluginInfo
    {
        QString libraryPath;
        QPluginLoader *loader;
        Qv2rayPlugin::Qv2rayInterface *pinterface;
        Q_ALWAYS_INLINE Qv2rayPlugin::QvPluginMetadata metadata() const
        {
            return pinterface->GetMetadata();
        }
        Q_ALWAYS_INLINE PluginId id() const
        {
            return metadata().InternalID;
        }
        Q_ALWAYS_INLINE bool hasComponent(Qv2rayPlugin::QV2RAY_PLUGIN_COMPONENT_TYPE t) const
        {
            return metadata().Components.contains(t);
        }
    };

    class PluginManagerCorePrivate
    {
      public:
        QHash<PluginId, PluginInfo> plugins;
    };

} // namespace Qv2rayBase::Plugin
