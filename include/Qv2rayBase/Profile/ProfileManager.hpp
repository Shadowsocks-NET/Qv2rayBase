#pragma once

#include "Common/Utils.hpp"
#include "Connections/ConnectionsBase.hpp"
#include "Qv2rayBaseFeatures.hpp"
#include "Qv2rayBase_export.h"

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
        bool IsValidId(const ConnectionGroupPair &id) const;

        bool IsConnected(const ConnectionGroupPair &id) const override;

        const GroupObject GetGroupObject(const GroupId &id) const override;
        const ConnectionObject GetConnectionObject(const ConnectionId &id) const override;

        const QList<ConnectionId> GetConnections() const override;
        const QList<ConnectionId> GetConnections(const GroupId &groupId) const override;

        const QList<GroupId> GetGroups() const override;
        const QList<GroupId> GetGroups(const ConnectionId &connId) const override;

        bool StartConnection(const ConnectionGroupPair &identifier) override;
        void StopConnection() override;

        const ConnectionGroupPair CreateConnection(const ProfileContent &root, const QString &displayName, const GroupId &groupId = DefaultGroupId) override;
        void UpdateConnection(const ConnectionId &id, const ProfileContent &root) override;
        void RenameConnection(const ConnectionId &id, const QString &newName) override;
        bool RemoveFromGroup(const ConnectionId &id, const GroupId &gid) override;
        bool MoveToGroup(const ConnectionId &id, const GroupId &sourceGid, const GroupId &targetGid) override;
        bool LinkWithGroup(const ConnectionId &id, const GroupId &newGroupId) override;

        const ProfileContent GetConnection(const ConnectionId &id) const override;

        const GroupId CreateGroup(const QString &displayName) override;
        const std::optional<QString> DeleteGroup(const GroupId &id) override;
        const std::optional<QString> RenameGroup(const GroupId &id, const QString &newName) override;
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
        void ClearGroupUsage(const GroupId &id) override;
        void ClearConnectionUsage(const ConnectionGroupPair &id) override;
      private slots:
        void p_OnStatsDataArrived(const ConnectionGroupPair &id, const QMap<Qv2rayBase::models::StatisticsType, Qv2rayBase::models::QvStatsSpeed> &data);
      signals:
        void OnStatsAvailable(const ConnectionGroupPair &id, const QMap<StatisticsType, QvStatsSpeedData> &data);
#endif

#if QV2RAYBASE_FEATURE(latency)
      public:
        void StartLatencyTest();
        void StartLatencyTest(const GroupId &id);
        void StartLatencyTest(const ConnectionId &id, Qv2rayLatencyTestingMethod method = GlobalConfig.networkConfig->latencyTestingMethod);
      signals:
        void OnLatencyTestStarted(const ConnectionId &id);
        void OnLatencyTestFinished(const ConnectionId &id, const int average);
      private slots:
        void p_OnLatencyDataArrived(const ConnectionId &id, const LatencyTestResult &data);
#endif

      signals:
        void OnConnectionCreated(const ConnectionGroupPair &Id, const QString &displayName);
        void OnConnectionModified(const ConnectionId &id);
        void OnConnectionRenamed(const ConnectionId &Id, const QString &originalName, const QString &newName);

        void OnConnectionLinkedWithGroup(const ConnectionGroupPair &newPair);
        void OnConnectionRemovedFromGroup(const ConnectionGroupPair &pairId);

        void OnGroupCreated(const GroupId &id, const QString &displayName);
        void OnGroupRenamed(const GroupId &id, const QString &oldName, const QString &newName);
        void OnGroupDeleted(const GroupId &id, const QList<ConnectionId> &connections);

        void OnConnected(const ConnectionGroupPair &id);
        void OnDisconnected(const ConnectionGroupPair &id);
        void OnKernelCrashed(const ConnectionGroupPair &id, const QString &errMessage);

      private slots:
        void p_OnKernelCrashed(const ConnectionGroupPair &id, const QString &errMessage);

      private:
        QScopedPointer<ProfileManagerPrivate> d_ptr;
        Q_DECLARE_PRIVATE(ProfileManager)
    };
} // namespace Qv2rayBase::Profile
