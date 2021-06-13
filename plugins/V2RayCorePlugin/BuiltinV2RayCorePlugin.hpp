#pragma once

#include <QObject>
#include <QtPlugin>
#include <QvPluginInterface.hpp>

using namespace Qv2rayPlugin;

namespace Qv2rayBase::BuiltinPlugins::V2RayCore
{
    class BuiltinV2RayCorePlugin
        : public QObject
        , Qv2rayInterface
    {
        Q_INTERFACES(Qv2rayPlugin::Qv2rayInterface)
        Q_PLUGIN_METADATA(IID Qv2rayInterface_IID)
        Q_OBJECT

      public:
        const QvPluginMetadata GetMetadata() const override;
        ~BuiltinV2RayCorePlugin();
        bool InitializePlugin() override;
        void SettingsUpdated() override;

      signals:
        void PluginLog(QString) const override;
        void PluginErrorMessageBox(QString, QString) const override;
    };
} // namespace Qv2rayBase::StaticPlugin
