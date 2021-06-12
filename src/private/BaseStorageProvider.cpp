#include "BaseStorageProvider.hpp"

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

class Qv2rayBase::_private::Qv2rayBasePrivateStorageProviderPrivate
{
  public:
    QString ConfigFilePath;
    QString ConfigDirPath;
    StorageContext RuntimeContext;
    QString ExecutableDirPath;
};

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

namespace Qv2rayBase::_private
{
    Qv2rayBasePrivateStorageProvider::Qv2rayBasePrivateStorageProvider(QObject *parent) : QObject(parent)
    {
    }

    bool Qv2rayBasePrivateStorageProvider::LookupConfigurations(StorageContext runtimeContext)
    {
        QStringList configDirPathsSearchList;

        {
            // Application directory
            configDirPathsSearchList << qApp->applicationDirPath() + "/config" + (runtimeContext.isDebug ? "_debug" : "");

            // Standard platform-independent configuration location
            configDirPathsSearchList << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qv2ray" + (runtimeContext.isDebug ? "_debug" : "");
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
        Q_D(Qv2rayBasePrivateStorageProvider);
        d->RuntimeContext = runtimeContext;
        d->ExecutableDirPath = qApp->applicationDirPath();
        d->ConfigFilePath = selectedConfigurationFile;
        d->ConfigDirPath = QFileInfo(d->ConfigFilePath).filePath();
        QvLog() << "Using" << selectedConfigurationFile << "as the config path.";
        return true;
    }

    QJsonObject Qv2rayBasePrivateStorageProvider::GetBaseConfiguration()
    {
        Q_D(Qv2rayBasePrivateStorageProvider);
        return JsonFromString(ReadFile(d->ConfigFilePath));
    }

    bool Qv2rayBasePrivateStorageProvider::StoreBaseConfiguration(const QJsonObject &json)
    {
        Q_D(Qv2rayBasePrivateStorageProvider);
        return WriteFile(JsonToString(json).toUtf8(), d->ConfigFilePath);
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

    QStringList Qv2rayBasePrivateStorageProvider::GetAssetsPath(const QString &)
    {
        return {};
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

} // namespace Qv2rayBase::_private
