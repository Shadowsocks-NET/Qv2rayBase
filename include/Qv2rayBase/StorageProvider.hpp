#pragma once
#include "Connections/ConnectionsBaseTypes.hpp"

#include <QDir>
#include <QJsonObject>

namespace Qv2rayBase
{
    class IStorageProvider
    {
      public:
        explicit IStorageProvider() = default;
        virtual ~IStorageProvider() = default;

        virtual bool LookupConfigurations() = 0;

        virtual QStringList GetAssetsPath(const QString &) = 0;

        virtual QJsonObject GetBaseConfiguration() = 0;
        virtual bool StoreBaseConfiguration(const QJsonObject &) = 0;

        virtual QHash<ConnectionId, ConnectionObject> Connections() = 0;
        virtual QHash<GroupId, GroupObject> Groups() = 0;
        virtual QHash<RoutingId, RoutingObject> Routings() = 0;

        virtual ProfileContent GetConnectionContent(const ConnectionId &) = 0;
        virtual bool StoreConnection(const ConnectionId &, ProfileContent) = 0;
        virtual bool DeleteConnection(const ConnectionId &) = 0;

        virtual QDir GetPluginWorkingDirectory(const PluginId &) = 0;
        virtual QJsonObject GetPluginSettings(const PluginId &) = 0;
        virtual void SetPluginSettings(const PluginId &, const QJsonObject &) = 0;
    };
} // namespace Qv2rayBase
