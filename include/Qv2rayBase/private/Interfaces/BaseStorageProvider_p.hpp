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

// ************************ WARNING ************************
//
// This file is NOT part of the Qv2rayBase API.
// It may change at any time without notice, or even be removed.
// USE IT AT YOUR OWN RISK
//
// ************************ WARNING ************************

#include "Interfaces/IStorageProvider.hpp"

#include <QObject>

namespace Qv2rayBase::Interfaces
{
    class Qv2rayBasePrivateStorageProvider
        : public QObject
        , public IStorageProvider
    {
        Q_OBJECT
      public:
        Qv2rayBasePrivateStorageProvider(QObject *parent = nullptr);
        virtual ~Qv2rayBasePrivateStorageProvider() = default;

        virtual bool LookupConfigurations(StorageContext) override;

        virtual QJsonObject GetBaseConfiguration() override;
        virtual bool StoreBaseConfiguration(const QJsonObject &) override;

        virtual QHash<ConnectionId, ConnectionObject> Connections() override;
        virtual QHash<GroupId, GroupObject> Groups() override;
        virtual QHash<RoutingId, RoutingObject> Routings() override;

        virtual ProfileContent GetConnectionContent(const ConnectionId &) override;
        virtual bool StoreConnection(const ConnectionId &, ProfileContent) override;
        virtual bool DeleteConnection(const ConnectionId &id) override;

        virtual QDir GetPluginWorkingDirectory(const PluginId &pid) override;

        virtual QJsonObject GetPluginSettings(const PluginId &pid) override;
        virtual void SetPluginSettings(const PluginId &pid, const QJsonObject &obj) override;

        virtual QStringList GetAssetsPath(const QString &) override;

      public:
        virtual void EnsureSaved() override;
        virtual void StoreConnections(const QHash<ConnectionId, ConnectionObject> &) override;
        virtual void StoreGroups(const QHash<GroupId, GroupObject> &) override;
        virtual void StoreRoutings(const QHash<RoutingId, RoutingObject> &) override;

      private:
        QString ConfigFilePath;
        QString ConfigDirPath;
        StorageContext RuntimeContext;
        QString ExecutableDirPath;
    };

} // namespace Qv2rayBase::Interfaces
