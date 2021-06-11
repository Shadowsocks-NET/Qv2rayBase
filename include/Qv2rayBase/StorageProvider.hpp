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

        virtual QJsonObject GetBaseConfiguration() = 0;

        virtual QHash<ConnectionId, ConnectionObject> Connections() = 0;
        virtual QHash<GroupId, GroupObject> Groups() = 0;
        virtual QHash<RoutingId, RoutingObject> Routings() = 0;

        virtual ProfileContent LoadConnection(const ConnectionId &) = 0;
        virtual bool StoreConnection(const ConnectionId &, ProfileContent) = 0;
        virtual bool DeleteConnection(const ConnectionId &id) = 0;

        virtual QDir GetPluginWorkingDirectory(const PluginId &pid) = 0;
        virtual QJsonObject GetPluginSettings(const PluginId &pid) = 0;
        virtual void SetPluginSettings(const PluginId &pid, const QJsonObject &obj) = 0;
    };
} // namespace Qv2rayBase
