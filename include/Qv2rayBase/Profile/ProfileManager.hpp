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

#include "Common/Utils.hpp"
#include "Connections/ConnectionsBase.hpp"
#include "Qv2rayBaseFeatures.hpp"
#include "QvPluginInterface.hpp"

namespace Qv2rayBase::Profile
{
    class ProfileManagerPrivate;
    class QV2RAYBASE_EXPORT ProfileManager
        : public QObject
        , public Qv2rayPlugin::Connections::IProfileManager
    {
        Q_OBJECT
      public:
        explicit ProfileManager(QObject *parent = nullptr);
        ~ProfileManager();
        void SaveConnectionConfig();

        bool IsValidId(const ConnectionId &id) const;
        bool IsValidId(const GroupId &id) const;
        bool IsValidId(const ProfileId &id) const;

        bool IsConnected(const ProfileId &id) const override;

        const GroupObject GetGroupObject(const GroupId &id) const override;
        const ConnectionObject GetConnectionObject(const ConnectionId &id) const override;

        const QList<ConnectionId> GetConnections() const override;
        const QList<ConnectionId> GetConnections(const GroupId &groupId) const override;

        const QList<GroupId> GetGroups() const override;
        const QList<GroupId> GetGroups(const ConnectionId &connId) const override;

        bool RestartConnection() override;
        bool StartConnection(const ProfileId &identifier) override;
        void StopConnection() override;

        const ProfileId CreateConnection(const ProfileContent &root, const QString &name, const GroupId &groupId = DefaultGroupId) override;
        void UpdateConnection(const ConnectionId &id, const ProfileContent &root) override;
        void RenameConnection(const ConnectionId &id, const QString &newName) override;
        bool RemoveFromGroup(const ConnectionId &id, const GroupId &gid) override;
        bool MoveToGroup(const ConnectionId &id, const GroupId &sourceGid, const GroupId &targetGid) override;
        bool LinkWithGroup(const ConnectionId &id, const GroupId &newGroupId) override;

        const ProfileContent GetConnection(const ConnectionId &id) const override;

        const GroupId CreateGroup(const QString &displayName) override;
        bool DeleteGroup(const GroupId &id, bool removeConnections) override;
        bool RenameGroup(const GroupId &id, const QString &newName) override;
        const RoutingId GetGroupRoutingId(const GroupId &id) override;

#if QV2RAYBASE_FEATURE(subscriptions)
      public:
        void IgnoreSubscriptionUpdate(const GroupId &group);
        void UpdateSubscriptionAsync(const GroupId &id);
        bool UpdateSubscription(const GroupId &id);
        void SetSubscriptionData(const GroupId &id, const SubscriptionConfigObject &config);
      signals:
        void OnSubscriptionAsyncUpdateFinished(const GroupId &id);
      private slots:
        bool p_CHUpdateSubscription(const GroupId &id, const QByteArray &data);
#endif

#if QV2RAYBASE_FEATURE(statistics)
      public:
        void ClearGroupUsage(const GroupId &id);
        void ClearConnectionUsage(const ProfileId &id);
      private slots:
        void p_OnStatsDataArrived(const ProfileId &id, const QMap<StatisticsObject::StatisticsType, StatisticsObject::StatsEntry> &speed);
      signals:
        void OnStatsAvailable(const ProfileId &id, const QMap<StatisticsObject::StatisticsType, StatisticsObject::StatsEntry> &speed);
#endif

#if QV2RAYBASE_FEATURE(latency)
      public:
        void StartLatencyTest(const ConnectionId &id, const LatencyTestEngineId &engine);
        void StartLatencyTest(const GroupId &id, const LatencyTestEngineId &engine);
      signals:
        void OnLatencyTestStarted(const ConnectionId &id);
        void OnLatencyTestFinished(const ConnectionId &id, const int average);
      private slots:
        void p_OnLatencyDataArrived(const ConnectionId &id, const Qv2rayPlugin::LatencyTestResponse &data);
#endif

      signals:
        void OnConnectionCreated(const ProfileId &Id, const QString &displayName);
        void OnConnectionModified(const ConnectionId &id);
        void OnConnectionRenamed(const ConnectionId &Id, const QString &originalName, const QString &newName);

        void OnConnectionLinkedWithGroup(const ProfileId &newPair);
        void OnConnectionRemovedFromGroup(const ProfileId &pairId);

        void OnGroupCreated(const GroupId &id, const QString &displayName);
        void OnGroupRenamed(const GroupId &id, const QString &oldName, const QString &newName);
        void OnGroupDeleted(const GroupId &id, const QList<ConnectionId> &connections);

        void OnConnected(const ProfileId &id);
        void OnDisconnected(const ProfileId &id);
        void OnKernelCrashed(const ProfileId &id, const QString &errMessage);

      private slots:
        void p_OnKernelCrashed(const ProfileId &id, const QString &errMessage);

      private:
        QScopedPointer<ProfileManagerPrivate> d_ptr;
        Q_DECLARE_PRIVATE(ProfileManager)
    };
} // namespace Qv2rayBase::Profile
