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

#include "private/Interfaces/BaseStorageProvider_p.hpp"

#include <Common/Utils.hpp>
#include <Qv2rayBaseLibrary.hpp>

#define QV_MODULE_NAME "BuiltinStorageProvider"

const auto QV2RAY_CONFIG_PATH_ENV_NAME = "QV2RAY_CONFIG_PATH";
const auto QV2RAY_CONFIG_FILE_NAME = "Qv2ray.conf";
const auto QV2RAY_CONFIG_CONNECTIONS_BASENAME = "connections";
const auto QV2RAY_CONFIG_GROUPS_BASENAME = "groups";
const auto QV2RAY_CONFIG_ROUTINGS_BASENAME = "routings";
const auto QV2RAY_CONFIG_PLUGINS_BASENAME = "plugins";
const auto QV2RAY_CONFIG_PLUGIN_SETTINGS_BASENAME = "plugin_settings";

#define DEBUG_SUFFIX (RuntimeContext.isDebug ? QStringLiteral("_debug/") : QStringLiteral("/"))

bool CheckSettingsPathAvailability(const QString &_dirPath, bool checkExistingConfig)
{
    auto path = _dirPath;
    if (!path.endsWith("/"))
        path.append("/");

    // Does not exist.
    if (!QDir(path).exists())
        return false;
    {
        // A temp file used to test file permissions in that folder.
        QFile testFile(path + ".qv2ray_test_file" + QString::number(QTime::currentTime().msecsSinceStartOfDay()));
        if (!testFile.open(QFile::OpenModeFlag::ReadWrite))
        {
            QvLog() << "Directory at:" << path << "cannot be used as a valid config file path.";
            QvLog() << "---> Cannot create a new file or open a file for writing.";
            return false;
        }
        testFile.write("Qv2ray test file, feel free to remove.");
        testFile.flush();
        testFile.close();
        if (!testFile.remove())
        {
            // This is rare, as we can create a file but failed to remove it.
            QvLog() << "Directory at:" << path << "cannot be used as a valid config file path.";
            QvLog() << "---> Cannot remove a file.";
            return false;
        }
    }

    if (!checkExistingConfig)
    {
        // Just pass the test
        return true;
    }

    QFile configFile(path + QV2RAY_CONFIG_FILE_NAME);

    // No such config file.
    if (!configFile.exists())
        return false;

    if (!configFile.open(QIODevice::ReadWrite))
    {
        QvLog() << "File:" << configFile.fileName() << " cannot be opened!";
        return false;
    }

    const auto err = VerifyJsonString(configFile.readAll());
    if (err)
    {
        QvLog() << "Json parse returns:" << *err;
        return false;
    }

    return true;
}

namespace Qv2rayBase::Interfaces
{
    Qv2rayBasePrivateStorageProvider::Qv2rayBasePrivateStorageProvider(QObject *parent) : QObject(parent)
    {
    }

    bool Qv2rayBasePrivateStorageProvider::LookupConfigurations(StorageContext runtimeContext)
    {
        QStringList configDirPathsSearchList;

        {
            // Application directory
            configDirPathsSearchList << qApp->applicationDirPath() + "/config" + DEBUG_SUFFIX;

            // Standard platform-independent configuration location
            configDirPathsSearchList << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qv2ray" + DEBUG_SUFFIX;
        }

        // Custom configuration path
        if (qEnvironmentVariableIsSet(QV2RAY_CONFIG_PATH_ENV_NAME))
        {
            const auto manualConfigPath = qEnvironmentVariable(QV2RAY_CONFIG_PATH_ENV_NAME);
            QvLog() << "Using config path from env:" << manualConfigPath;
            configDirPathsSearchList.clear();
            configDirPathsSearchList << manualConfigPath;
        }

        QString selectedConfigurationFile;

        for (const auto &dirPath : configDirPathsSearchList)
        {
            // Verify the config path, check if the config file exists and in the correct JSON format.
            // True means we check for config existence as well. --------|HERE|
            bool isValidConfigPath = CheckSettingsPathAvailability(dirPath, true);

            if (isValidConfigPath)
            {
                QvDebug() << "Path:" << dirPath << "is valid.";
                selectedConfigurationFile = dirPath + QV2RAY_CONFIG_FILE_NAME;
                break;
            }
            QvLog() << "Path:" << dirPath << "does not contain a valid config file.";
        }

        if (selectedConfigurationFile.isEmpty())
        {
            // If there's no existing config, use the first one in the search list.
            selectedConfigurationFile = configDirPathsSearchList.first() + QV2RAY_CONFIG_FILE_NAME;

            // Check if the dirs are writeable
            const auto hasPossibleNewLocation = QDir().mkpath(selectedConfigurationFile) && CheckSettingsPathAvailability(selectedConfigurationFile, false);
            if (!hasPossibleNewLocation)
            {
                // None of the path above can be used as a dir for storing config.
                // Even the last folder failed to pass the check.
                QvLog() << "FATAL:";
                QvLog() << "Cannot load configuration file Qv2ray";
                QvLog() << "Cannot find a place to store config files." << NEWLINE << "Qv2ray has searched these paths below:";
                QvLog() << "";
                QvLog() << configDirPathsSearchList;
                QvLog() << "It usually means you don't have the write permission to all of those locations.";
                return false;
            }

            // Found a valid config dir, with write permission, but assume no config is located in it.
            if (QFile::exists(selectedConfigurationFile))
            {
                // As we already tried to load config from every possible dir.
                //
                // This condition branch (!hasExistingConfig check) holds the fact that current config dir,
                // should NOT contain any valid file (at least in the same name)
                //
                // It usually means that QV2RAY_CONFIG_FILE here has a corrupted JSON format.
                //
                // Otherwise Qv2ray would have loaded this config already instead of notifying to create a new config in this folder.
                //
                QvLog() << "This should not occur: Qv2ray config exists but cannot be load.";
                QvLog() << "Failed to initialise Qv2rayBase";
                QvLog() << "Qv2ray has found a config file, but it failed to be loaded due to some errors.";
                QvLog() << "A workaround is to remove the this file and restart Qv2ray:";
                QvLog() << selectedConfigurationFile;
                return false;
            }

            // Now make the file exist.
            WriteFile(R"({ "dummy": "Hello, Qv2ray!" })", selectedConfigurationFile);
        }

        // At this step, the "selectedConfigurationFile" is ensured to be OK for storing configuration.
        // Use the config path found by the checks above
        RuntimeContext = runtimeContext;
        ExecutableDirPath = qApp->applicationDirPath();
        ConfigFilePath = selectedConfigurationFile;
        ConfigDirPath = QFileInfo(ConfigFilePath).filePath();
        QvLog() << "Using" << selectedConfigurationFile << "as the config path.";
        return true;
    }

    QJsonObject Qv2rayBasePrivateStorageProvider::GetBaseConfiguration()
    {
        return JsonFromString(ReadFile(ConfigFilePath));
    }

    bool Qv2rayBasePrivateStorageProvider::StoreBaseConfiguration(const QJsonObject &json)
    {
        return WriteFile(JsonToString(json).toUtf8(), ConfigFilePath);
    }

    QHash<ConnectionId, ConnectionObject> Qv2rayBasePrivateStorageProvider::Connections()
    {
        return {};
    }

    QHash<GroupId, GroupObject> Qv2rayBasePrivateStorageProvider::Groups()
    {
        return {};
    }

    QHash<RoutingId, RoutingObject> Qv2rayBasePrivateStorageProvider::Routings()
    {
        return {};
    }

    ProfileContent Qv2rayBasePrivateStorageProvider::GetConnectionContent(const ConnectionId &)
    {
        return {};
    }

    bool Qv2rayBasePrivateStorageProvider::StoreConnection(const ConnectionId &, ProfileContent)
    {
        return {};
    }

    bool Qv2rayBasePrivateStorageProvider::DeleteConnection(const ConnectionId &id)
    {
        return {};
    }

    QDir Qv2rayBasePrivateStorageProvider::GetUserPluginDirectory()
    {
        return {};
    }

    QDir Qv2rayBasePrivateStorageProvider::GetPluginWorkingDirectory(const PluginId &pid)
    {
        return {};
    }

    QJsonObject Qv2rayBasePrivateStorageProvider::GetPluginSettings(const PluginId &pid)
    {
        return {};
    }

    void Qv2rayBasePrivateStorageProvider::SetPluginSettings(const PluginId &pid, const QJsonObject &obj)
    {
    }

    QStringList Qv2rayBasePrivateStorageProvider::GetAssetsPath(const QString &dirName)
    {
        static const auto makeAbs = [](const QDir &p) { return p.absolutePath(); };

        QStringList list;
        // Default behavior on Windows
        list << makeAbs(QCoreApplication::applicationDirPath() + "/" + dirName);

        // WARNING BREAKING CHANGE
        // list << makeAbs(instance()->d_ptr->configurationPaths + dirName);
        list << ":/" + dirName;

        list << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, dirName, QStandardPaths::LocateDirectory);
        list << QStandardPaths::locateAll(QStandardPaths::AppConfigLocation, dirName, QStandardPaths::LocateDirectory);

#ifdef Q_OS_UNIX
        if (qEnvironmentVariableIsSet("APPIMAGE"))
            list << makeAbs(QCoreApplication::applicationDirPath() + "/../share/qv2ray" + DEBUG_SUFFIX + dirName);

        if (qEnvironmentVariableIsSet("SNAP"))
            list << makeAbs(qEnvironmentVariable("SNAP") + "/usr/share/qv2ray" + DEBUG_SUFFIX + dirName);

        if (qEnvironmentVariableIsSet("XDG_DATA_DIRS"))
            list << makeAbs(qEnvironmentVariable("XDG_DATA_DIRS") + "/" + dirName);

        list << makeAbs("/usr/local/share/qv2ray" + DEBUG_SUFFIX + dirName);
        list << makeAbs("/usr/local/lib/qv2ray" + DEBUG_SUFFIX + dirName);
        list << makeAbs("/usr/share/qv2ray" + DEBUG_SUFFIX + dirName);
        list << makeAbs("/usr/lib/qv2ray" + DEBUG_SUFFIX + dirName);
        list << makeAbs("/lib/qv2ray" + DEBUG_SUFFIX + dirName);
#endif

#ifdef Q_OS_MAC
        // macOS platform directories.
        list << QDir(QCoreApplication::applicationDirPath() + "/../Resources/" + dirName).absolutePath();
#endif
        return list;
    }

    void Qv2rayBasePrivateStorageProvider::EnsureSaved()
    {
    }

    void Qv2rayBasePrivateStorageProvider::StoreConnections(const QHash<ConnectionId, ConnectionObject> &)
    {
    }

    void Qv2rayBasePrivateStorageProvider::StoreGroups(const QHash<GroupId, GroupObject> &)
    {
    }

    void Qv2rayBasePrivateStorageProvider::StoreRoutings(const QHash<RoutingId, RoutingObject> &)
    {
    }

} // namespace Qv2rayBase::Interfaces
