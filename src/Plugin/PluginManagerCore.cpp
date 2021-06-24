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

#include "Plugin/PluginManagerCore.hpp"

#include "Common/Settings.hpp"
#include "Common/Utils.hpp"
#include "Interfaces/IStorageProvider.hpp"
#include "Profile/ProfileManager.hpp"
#include "Qv2rayBaseLibrary.hpp"
#include "private/Plugin/PluginManagerCore_p.hpp"

#include <QDir>

#define QV_MODULE_NAME "PluginManagerCore"

namespace Qv2rayBase::Plugin
{
    using namespace Qv2rayPlugin;
    PluginManagerCore::PluginManagerCore(QObject *parent) : QObject(parent)
    {
        d_ptr.reset(new PluginManagerCorePrivate);
    }

    PluginManagerCore::~PluginManagerCore()
    {
        Q_D(PluginManagerCore);
        for (auto &&plugin : d->plugins)
        {
            QvDebug() << "Unloading plugin:" << plugin.metadata().Name;

            // Static plugins doesn't have a loader.
            if (plugin.loader)
            {
                plugin.loader->unload();
                plugin.loader->deleteLater();
            }
        }
        d->plugins.clear();
    }

    void PluginManagerCore::LoadPlugins()
    {
        Q_D(PluginManagerCore);
        QvLog() << "Reloading plugin list";
        for (const auto &pluginDirPath : Qv2rayBaseLibrary::GetAssetsPaths(QStringLiteral("plugins")))
        {
            const auto entries = QDir(pluginDirPath).entryList(QDir::Files);
            for (const auto &fileName : entries)
            {
                tryLoadPlugin(QDir(pluginDirPath).absoluteFilePath(fileName));
            }
        }

        for (const auto &plugin : QPluginLoader::staticInstances())
        {
            loadPluginImpl(QStringLiteral("[STATIC]"), plugin, nullptr);
        }

        for (auto &plugin : d->plugins.keys())
        {
            auto wd = Qv2rayBaseLibrary::StorageProvider()->GetPluginWorkingDirectory(plugin);
            auto conf = Qv2rayBaseLibrary::StorageProvider()->GetPluginSettings(plugin);

            d->plugins[plugin].pinterface->m_Settings = conf;
            d->plugins[plugin].pinterface->m_WorkingDirectory.setPath(wd.absolutePath());
            d->plugins[plugin].pinterface->m_ConnectionManager = Qv2rayBaseLibrary::ProfileManager();
            d->plugins[plugin].pinterface->InitializePlugin();
        }
    }

    void PluginManagerCore::SetPluginSettings(const PluginId &pid, const QJsonObject &settings)
    {
        const auto plugin = GetPlugin(pid);
        plugin->pinterface->m_Settings = settings;
        if (GetPluginEnabled(pid))
            plugin->pinterface->SettingsUpdated();
    }

    const QList<const PluginInfo *> PluginManagerCore::AllPlugins() const
    {
        Q_D(const PluginManagerCore);
        QList<const PluginInfo *> list;
        list.reserve(d->plugins.size());
        for (const auto &plugin : d->plugins)
            list << &plugin;
        return list;
    }

    QList<const PluginInfo *> PluginManagerCore::GetPlugins(Qv2rayPlugin::PLUGIN_COMPONENT_TYPE c) const
    {
        Q_D(const PluginManagerCore);
        QList<const PluginInfo *> list;
        for (const auto &p : qAsConst(d->plugins))
        {
            if (GetPluginEnabled(p.id()) && p.hasComponent(c))
                list << &p;
        }
        return list;
    }

    const PluginInfo *PluginManagerCore::GetPlugin(const PluginId &internalName)
    {
        Q_D(PluginManagerCore);
        return !d->plugins.isEmpty() && d->plugins.contains(internalName) ? &d->plugins[internalName] : nullptr;
    }

    bool PluginManagerCore::tryLoadPlugin(const QString &pluginFullPath)
    {
        if (!pluginFullPath.endsWith(QStringLiteral(".dll")) && !pluginFullPath.endsWith(QStringLiteral(".so")) && !pluginFullPath.endsWith(QStringLiteral(".dylib")))
            return false;

        if (pluginFullPath.isEmpty())
            return false;

        auto loader = new QPluginLoader(pluginFullPath, this);

        QObject *instance = loader->instance();
        if (!instance)
        {
            const auto errMessage = loader->errorString();
            QvLog() << errMessage;
            Qv2rayBaseLibrary::Warn(tr("Failed to load plugin"), errMessage);
            return false;
        }

        return loadPluginImpl(pluginFullPath, instance, loader);
    }

    void PluginManagerCore::PluginLog(QString log)
    {
        if (auto _interface = qobject_cast<Qv2rayInterface *>(sender()); _interface)
        {
            QvLog() << _interface->GetMetadata().InternalID << ":" << log;
        }
        else
        {
            QvLog() << "Unknown Plugin:" << log;
        }
    }

    void PluginManagerCore::PluginMessageBox(QString title, QString message)
    {
        const auto pInterface = qobject_cast<Qv2rayInterface *>(sender());
        if (pInterface)
            Qv2rayBaseLibrary::Warn(pInterface->GetMetadata().Name + " - " + title, message);
        else
            Qv2rayBaseLibrary::Warn("Unknown Plugin - " + title, message);
    }

    bool PluginManagerCore::GetPluginEnabled(const PluginId &pid) const
    {
        return Qv2rayBaseLibrary::GetConfig()->plugin_config.plugin_states.value(pid.toString(), true);
    }

    void PluginManagerCore::SetPluginEnabled(const PluginId &pid, bool isEnabled) const
    {
        Qv2rayBaseLibrary::GetConfig()->plugin_config.plugin_states.insert(pid.toString(), isEnabled);
    }

    void PluginManagerCore::SavePluginSettings() const
    {
        Q_D(const PluginManagerCore);
        for (const auto &name : d->plugins.keys())
            Qv2rayBaseLibrary::StorageProvider()->SetPluginSettings(name, d->plugins[name].pinterface->m_Settings);
    }

    bool PluginManagerCore::loadPluginImpl(const QString &fullPath, QObject *instance, QPluginLoader *loader)
    {
        Q_D(PluginManagerCore);

        PluginInfo info;
        info.libraryPath = fullPath;
        info.loader = loader;
        info.pinterface = qobject_cast<Qv2rayInterface *>(instance);

        if (!info.pinterface)
        {
            QvLog() << "Failed to cast from QObject to Qv2rayPluginInterface";
            // Static plugins doesn't have a loader.
            if (info.loader)
            {
                info.loader->unload();
                info.loader->deleteLater();
            }
            return false;
        }

        if (info.pinterface->QvPluginInterfaceVersion != QV2RAY_PLUGIN_INTERFACE_VERSION)
        {
            // The plugin was built for a not-compactable version of Qv2ray. Don't load the plugin by default.
            QvLog() << info.libraryPath << " is built with an older Interface, ignoring";
            Qv2rayBaseLibrary::Warn(tr("Cannot load plugin"), tr("The plugin cannot be loaded: ") + "\n" + info.libraryPath + "\n\n" +
                                                                  tr("This plugin was built against a different version of the Plugin Interface.") + "\n" +
                                                                  tr("Please contact the plugin provider or report the issue to Qv2ray Workgroup."));
            info.loader->unload();
            return false;
        }

        if (d->plugins.contains(info.metadata().InternalID))
        {
            QvLog() << "Found another plugin with the same internal name:" << info.metadata().InternalID << ". Skipped";
            return false;
        }

        // Normalized function signature should not contain a space char, which would be added by clang-format
        // clang-format off
        connect(instance, SIGNAL(PluginLog(QString)), this, SLOT(QvPluginLog(QString)));
        connect(instance, SIGNAL(PluginErrorMessageBox(QString,QString)), this, SLOT(QvPluginMessageBox(QString,QString)));
        // clang-format on

        QvLog() << "Loaded plugin:" << info.metadata().Name << "made by:" << info.metadata().Author;
        d->plugins.insert(info.metadata().InternalID, info);
        return true;
    }
} // namespace Qv2rayBase::Plugin
