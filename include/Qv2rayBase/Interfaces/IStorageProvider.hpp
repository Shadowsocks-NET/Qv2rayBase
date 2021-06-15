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
#include "Common/CommonTypes.hpp"

#include <QDir>

namespace Qv2rayBase::Interfaces
{
    struct StorageContext
    {
        bool isDebug;
    };

    class IStorageProvider
    {
      public:
        explicit IStorageProvider() = default;
        virtual ~IStorageProvider() = default;

        virtual bool LookupConfigurations(StorageContext) = 0;
        virtual void EnsureSaved() = 0;

        virtual QStringList GetAssetsPath(const QString &) = 0;

        virtual QJsonObject GetBaseConfiguration() = 0;
        virtual bool StoreBaseConfiguration(const QJsonObject &) = 0;

        virtual QHash<ConnectionId, ConnectionObject> Connections() = 0;
        virtual QHash<GroupId, GroupObject> Groups() = 0;
        virtual QHash<RoutingId, RoutingObject> Routings() = 0;

        virtual void StoreConnections(const QHash<ConnectionId, ConnectionObject> &) = 0;
        virtual void StoreGroups(const QHash<GroupId, GroupObject> &) = 0;
        virtual void StoreRoutings(const QHash<RoutingId, RoutingObject> &) = 0;

        virtual ProfileContent GetConnectionContent(const ConnectionId &) = 0;
        virtual bool StoreConnection(const ConnectionId &, ProfileContent) = 0;
        virtual bool DeleteConnection(const ConnectionId &) = 0;

        virtual QDir GetPluginWorkingDirectory(const PluginId &) = 0;
        virtual QJsonObject GetPluginSettings(const PluginId &) = 0;
        virtual void SetPluginSettings(const PluginId &, const QJsonObject &) = 0;
    };
} // namespace Qv2rayBase::Interfaces
