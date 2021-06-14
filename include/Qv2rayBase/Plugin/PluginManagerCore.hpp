#pragma once

#include "Qv2rayBase_export.h"
#include "QvPluginInterface.hpp"

#include <QObject>

class QPluginLoader;

namespace Qv2rayBase::Plugin
{
    struct PluginInfo;

    class PluginManagerCorePrivate;
    class QV2RAYBASE_EXPORT PluginManagerCore : public QObject
    {
        Q_OBJECT
      public:
        PluginManagerCore(QObject *parent = nullptr);
        virtual ~PluginManagerCore();

        void LoadPlugins();
        void SetPluginSettings(const PluginId &pid, const QJsonObject &settings);

        QList<const PluginInfo *> GetPlugins(Qv2rayPlugin::QV2RAY_PLUGIN_COMPONENT_TYPE c) const;
        const QList<const PluginInfo *> AllPlugins() const;
        const PluginInfo *GetPlugin(const PluginId &pid);

        bool GetPluginEnabled(const PluginId &pid) const;
        void SetPluginEnabled(const PluginId &pid, bool isEnabled) const;

      private:
        bool tryLoadPlugin(const QString &pluginFullPath);
        void SavePluginSettings() const;

      private slots:
        void QvPluginLog(QString log);
        void QvPluginMessageBox(QString title, QString message);

      private:
        Q_DECLARE_PRIVATE(PluginManagerCore)
    };
} // namespace Qv2rayBase::Plugin
