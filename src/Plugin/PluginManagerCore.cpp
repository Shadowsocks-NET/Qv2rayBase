#include "Plugin/PluginManagerCore.hpp"

#include "Common/Utils.hpp"
#include "Models/Settings.hpp"
#include "Profile/ProfileManager.hpp"
#include "Qv2rayBaseLibrary.hpp"
#include "StorageProvider.hpp"

#include <QDir>
#include <QPluginLoader>

using namespace Qv2rayPlugin;

#define QV_MODULE_NAME "PluginManagerCore"

namespace Qv2rayBase::Plugins
{
    class PluginManagerCorePrivate
    {
      public:
        QHash<PluginId, PluginInfo> plugins;
    };

    PluginManagerCore::PluginManagerCore(QObject *parent) : QObject(parent)
    {
    }

    PluginManagerCore::~PluginManagerCore()
    {
        Q_D(PluginManagerCore);
        SavePluginSettings();
        for (auto &&plugin : d->plugins)
        {
            DEBUG("Unloading: \"" + plugin.metadata().Name + "\"");
            plugin.loader->unload();
            plugin.loader->deleteLater();
        }
        d->plugins.clear();
    }

    void PluginManagerCore::LoadPlugins()
    {
        Q_D(PluginManagerCore);
        LOG("Reloading plugin list");
        for (const auto &pluginDirPath : Qv2rayBaseLibrary::GetAssetsPaths("plugins"))
        {
            const auto entries = QDir(pluginDirPath).entryList(QDir::Files);
            for (const auto &fileName : entries)
            {
                tryLoadPlugin(QDir(pluginDirPath).absoluteFilePath(fileName));
            }
        }

        for (auto &plugin : d->plugins.keys())
        {
            auto wd = Qv2rayBaseLibrary::StorageProvider()->GetPluginWorkingDirectory(plugin);
            auto conf = Qv2rayBaseLibrary::StorageProvider()->GetPluginSettings(plugin);

            d->plugins[plugin].pinterface->m_Settings = conf;
            d->plugins[plugin].pinterface->m_WorkingDirectory = wd;
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
        for (const auto &plugin : d->plugins)
            list << &plugin;
        return list;
    }
    QList<const PluginInfo *> PluginManagerCore::GetPlugins(Qv2rayPlugin::QV2RAY_PLUGIN_COMPONENT_TYPE c) const
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
        return d->plugins.contains(internalName) ? &d->plugins[internalName] : nullptr;
    }

    bool PluginManagerCore::tryLoadPlugin(const QString &pluginFullPath)
    {
        Q_D(PluginManagerCore);
        if (!pluginFullPath.endsWith(".dll") && !pluginFullPath.endsWith(".so") && !pluginFullPath.endsWith(".dylib"))
            return false;

        if (pluginFullPath.isEmpty())
            return false;

        PluginInfo info;
        info.libraryPath = pluginFullPath;
        info.loader = new QPluginLoader(pluginFullPath, this);

        QObject *plugin = info.loader->instance();
        if (!plugin)
        {
            const auto errMessage = info.loader->errorString();
            LOG(errMessage);
            Qv2rayBaseLibrary::Warn(tr("Failed to load plugin"), errMessage);
            return false;
        }

        info.pinterface = qobject_cast<Qv2rayInterface *>(plugin);
        if (!info.pinterface)
        {
            LOG("Failed to cast from QObject to Qv2rayPluginInterface");
            info.loader->unload();
            return false;
        }

        if (info.pinterface->QvPluginInterfaceVersion != QV2RAY_PLUGIN_INTERFACE_VERSION)
        {
            // The plugin was built for a not-compactable version of Qv2ray. Don't load the plugin by default.
            LOG(info.libraryPath + " is built with an older Interface, ignoring");
            Qv2rayBaseLibrary::Warn(tr("Cannot load plugin"), tr("The plugin cannot be loaded: ") + "\n" + info.libraryPath + "\n\n" +
                                                                  tr("This plugin was built against a different version of the Plugin Interface.") + "\n" +
                                                                  tr("Please contact the plugin provider or report the issue to Qv2ray Workgroup."));
            info.loader->unload();
            return false;
        }

        if (d->plugins.contains(info.metadata().InternalID))
        {
            LOG("Found another plugin with the same internal name: " + info.metadata().InternalID + ". Skipped");
            return false;
        }

        // Normalized function signature should not contain a space char, which would be added by clang-format
        // clang-format off
        connect(plugin, SIGNAL(PluginLog(QString)), this, SLOT(QvPluginLog(QString)));
        connect(plugin, SIGNAL(PluginErrorMessageBox(QString,QString)), this, SLOT(QvPluginMessageBox(QString,QString)));
        // clang-format on

        LOG("Loaded plugin: \"" + info.metadata().Name + "\" made by: \"" + info.metadata().Author + "\"");
        d->plugins.insert(info.metadata().InternalID, info);
        return true;
    }

    void PluginManagerCore::QvPluginLog(QString log)
    {
        if (auto _interface = qobject_cast<Qv2rayInterface *>(sender()); _interface)
        {
            LOG(_interface->GetMetadata().InternalID, log);
        }
        else
        {
            LOG("Unknown Plugin:", log);
        }
    }

    void PluginManagerCore::QvPluginMessageBox(QString title, QString message)
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
        {
            Qv2rayBaseLibrary::StorageProvider()->SetPluginSettings(name, d->plugins[name].pinterface->m_Settings);
        }
    }
} // namespace Qv2rayBase::Plugins
