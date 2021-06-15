//  Qv2rayBase, the modular feature-rich infrastructure library for Qv2ray.
//  Copyright (C) 2021 Moody and relavent Qv2ray contributors.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
