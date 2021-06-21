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

#include "Qv2rayBaseLibrary.hpp"

#include "Common/Settings.hpp"
#include "Plugin/LatencyTestHost.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Plugin/PluginManagerCore.hpp"
#include "Profile/KernelManager.hpp"
#include "Profile/ProfileManager.hpp"
#include "private/Common/SettingsUpgrade_p.hpp"
#include "private/Qv2rayBaseLibrary_p.hpp"

// Private headers
#include "private/Interfaces/BaseConfigurationGenerator_p.hpp"
#include "private/Interfaces/BaseStorageProvider_p.hpp"

#include <QDir>
#include <QStandardPaths>

// WARN Should we use this?
#include <QCoreApplication>

#define QV_MODULE_NAME "BaseApplication"

namespace Qv2rayBase
{
    using namespace Qv2rayBase::Profile;
    using namespace Qv2rayBase::Plugin;
    Qv2rayBaseLibrary *m_instance = nullptr;

    QV2RAYBASE_FAILED_REASON Qv2rayBaseLibrary::Initialize(Qv2rayStartFlags flags,                    //
                                                           Interfaces::IUserInteractionInterface *ui, //
                                                           Interfaces::IConfigurationGenerator *gen,  //
                                                           Interfaces::IStorageProvider *stor)
    {
        Q_ASSERT_X(m_instance == nullptr, "Qv2rayBaseLibrary", "m_instance is not null! Cannot construct another Qv2rayBaseLibrary when there's one existed");
        m_instance = this;
        Q_D(Qv2rayBaseLibrary);
        d->startupFlags = flags;

        d->uiInterface = ui;

        if (stor)
            d->storageProvider = stor;
        else
            d->storageProvider = new Interfaces::Qv2rayBasePrivateStorageProvider;

        if (gen)
            d->configGenerator = gen;
        else
            d->configGenerator = new Interfaces::Qv2rayBasePrivateConfigurationGenerator;

        d->configuration = new Models::Qv2rayBaseConfigObject;

#pragma message("TODO: Storage Context")
        if (!d->storageProvider->LookupConfigurations({}))
        {
            m_instance = nullptr;
            return ERR_LOCATE_CONFIGURATION;
        }

        QJsonObject configuration = _private::MigrateSettings(d->storageProvider->GetBaseConfiguration());
        d->configuration->loadJson(configuration);

        d->pluginCore = new Plugin::PluginManagerCore;
        d->pluginAPIHost = new Plugin::PluginAPIHost;

        if (!flags.testFlag(START_NO_PLUGINS))
            d->pluginCore->LoadPlugins();

        d->latencyTestHost = new Plugin::LatencyTestHost;
        d->profileManager = new Profile::ProfileManager;
        d->kernelManager = new Profile::KernelManager;

        return NORMAL;
    }

    void Qv2rayBaseLibrary::Shutdown()
    {
        Q_D(Qv2rayBaseLibrary);
        d->kernelManager->StopConnection();
        d->profileManager->SaveConnectionConfig();
        d->latencyTestHost->StopAllLatencyTest();
        d->pluginCore->SavePluginSettings();

        delete d->kernelManager;
        delete d->latencyTestHost;
        delete d->pluginAPIHost;
        delete d->pluginCore;
        delete d->profileManager;

        d->storageProvider->StoreBaseConfiguration(d->configuration->toJson());
        delete d->configuration;

        delete d->storageProvider;
        delete d->configGenerator;

        // delete d->uiInterface;
        m_instance = nullptr;
    }

    Qv2rayBaseLibrary *Qv2rayBaseLibrary::instance()
    {
        Q_ASSERT_X(m_instance, "Qv2rayBaseLibrary", "m_instance is null! Did you forget to initialize Qv2rayBase?");
        return m_instance;
    }

    Qv2rayBaseLibrary::Qv2rayBaseLibrary() : d_ptr(new Qv2rayBaseLibraryPrivate)
    {
        QvLog() << "Qv2ray Base Library" << QV2RAY_BASELIB_VERSION << "on" << QSysInfo::prettyProductName() << QSysInfo::currentCpuArchitecture();
        QvDebug() << "Qv2ray Start Time:" << QTime::currentTime().msecsSinceStartOfDay();
    }

    Qv2rayBaseLibrary::~Qv2rayBaseLibrary()
    {
    }

    QStringList Qv2rayBaseLibrary::GetAssetsPaths(const QString &dirName)
    {
        static const auto makeAbs = [](const QDir &p) { return p.absolutePath(); };

        // Configuration Path
        QStringList list;

        if (qEnvironmentVariableIsSet("QV2RAYBASE_RESOURCES_PATH"))
            list << makeAbs(qEnvironmentVariable("QV2RAYBASE_RESOURCES_PATH") + "/" + dirName);

        list << instance()->StorageProvider()->GetAssetsPath(dirName);
        list.removeDuplicates();
        return list;
    }

    void Qv2rayBaseLibrary::Warn(const QString &title, const QString &text)
    {
        instance()->d_ptr->uiInterface->p_MessageBoxWarn(title, text);
    }

    void Qv2rayBaseLibrary::Info(const QString &title, const QString &text)
    {
        instance()->d_ptr->uiInterface->p_MessageBoxInfo(title, text);
    }

    MessageOpt Qv2rayBaseLibrary::Ask(const QString &title, const QString &text, const QList<MessageOpt> &options)
    {
        return instance()->d_ptr->uiInterface->p_MessageBoxAsk(title, text, options);
    }

    void Qv2rayBaseLibrary::OpenURL(const QUrl &url)
    {
        instance()->d_ptr->uiInterface->p_OpenURL(url);
    }

    PluginAPIHost *Qv2rayBaseLibrary::PluginAPIHost()
    {
        return instance()->d_ptr->pluginAPIHost;
    }

    Models::Qv2rayBaseConfigObject *Qv2rayBaseLibrary::GetConfig()
    {
        return instance()->d_ptr->configuration;
    }

    PluginManagerCore *Qv2rayBaseLibrary::PluginManagerCore()
    {
        return instance()->d_ptr->pluginCore;
    }

    LatencyTestHost *Qv2rayBaseLibrary::LatencyTestHost()
    {
        return instance()->d_ptr->latencyTestHost;
    }

    Interfaces::IConfigurationGenerator *Qv2rayBaseLibrary::ConfigurationGenerator()
    {
        return instance()->d_ptr->configGenerator;
    }

    Interfaces::IStorageProvider *Qv2rayBaseLibrary::StorageProvider()
    {
        return instance()->d_ptr->storageProvider;
    }

    ProfileManager *Qv2rayBaseLibrary::ProfileManager()
    {
        return instance()->d_ptr->profileManager;
    }

    KernelManager *Qv2rayBaseLibrary::KernelManager()
    {
        return instance()->d_ptr->kernelManager;
    }
} // namespace Qv2rayBase
