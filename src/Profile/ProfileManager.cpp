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

#include "Profile/ProfileManager.hpp"

#include "Common/HTTPRequestHelper.hpp"
#include "Common/Utils.hpp"
#include "Interfaces/IStorageProvider.hpp"
#include "Plugin/LatencyTestHost.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/KernelManager.hpp"
#include "private/Profile/ProfileManager_p.hpp"

#include <QTimerEvent>

#define QV_MODULE_NAME "ConfigHandler"
#define CheckValidId(id, returnValue)                                                                                                                                    \
    if (!IsValidId(id))                                                                                                                                                  \
        return returnValue;

#define nothing

namespace Qv2rayBase::Profile
{
    using namespace Qv2rayPlugin::Event;
    using namespace Qv2rayBase::Utils;

    ProfileManager::ProfileManager(QObject *parent) : QObject(parent)
    {
        d_ptr.reset(new ProfileManagerPrivate);
        Q_D(ProfileManager);
        QvDebug() << "ProfileManager Constructor.";

        d->connections = Qv2rayBaseLibrary::StorageProvider()->GetConnections();
        const auto _groups = Qv2rayBaseLibrary::StorageProvider()->GetGroups();
        const auto _routings = Qv2rayBaseLibrary::StorageProvider()->GetRoutings();

        for (auto it = _groups.constKeyValueBegin(); it != _groups.constKeyValueEnd(); it++)
        {
            auto id = it->first;
            auto grp = it->second;
            if (grp.name.isEmpty())
                grp.name = tr("Group: %1").arg(GenerateRandomString(5));

            d->groups.insert(id, grp);

            for (const auto &connId : grp.connections)
            {
                d->connections[connId]._group_ref++;
            }
        }

        for (auto it = _routings.constKeyValueBegin(); it != _routings.constKeyValueEnd(); it++)
        {
            d->routings.insert(it->first, it->second);
        }

        for (auto it = d->connections.constKeyValueBegin(); it != d->connections.constKeyValueEnd(); it++)
        {
            auto id = it->first;
            auto conn = it->second;
            if (d->connections[id]._group_ref == 0)
            {
                d->connections.remove(id);
                Qv2rayBaseLibrary::StorageProvider()->DeleteConnection(id);
                QvLog() << "Dropped connection id:" << id << "since it's not in a group";
            }
            else
            {
                d->connectionRootCache[id] = Qv2rayBaseLibrary::StorageProvider()->GetConnectionContent(id);
                QvDebug() << "Loaded connection id:" << id << "into cache.";
            }
        }

        // Force default group name.
        if (!d->groups.contains(DefaultGroupId))
        {
            d->groups.insert(DefaultGroupId, GroupObject{});
            d->groups[DefaultGroupId].name = tr("Default Group");
        }
    }

    void ProfileManager::SaveConnectionConfig()
    {
        Q_D(ProfileManager);
        Qv2rayBaseLibrary::StorageProvider()->StoreConnections(d->connections);
        Qv2rayBaseLibrary::StorageProvider()->StoreGroups(d->groups);
        Qv2rayBaseLibrary::StorageProvider()->StoreRoutings(d->routings);
        Qv2rayBaseLibrary::StorageProvider()->EnsureSaved();
    }

#if QV2RAYBASE_FEATURE(latency)
    void ProfileManager::StartLatencyTest(const GroupId &id, const LatencyTestEngineId &engine)
    {
        Q_D(ProfileManager);
        for (const auto &connection : d->groups.value(id).connections)
            StartLatencyTest(connection, engine);
    }

    void ProfileManager::StartLatencyTest(const ConnectionId &id, const LatencyTestEngineId &engine)
    {
        emit OnLatencyTestStarted(id);
        Qv2rayBaseLibrary::LatencyTestHost()->TestLatency(id, engine);
    }
#endif

#if QV2RAYBASE_FEATURE(statistics)
    void ProfileManager::ClearGroupUsage(const GroupId &id)
    {
        Q_D(ProfileManager);
        for (const auto &conn : d->groups.value(id).connections)
        {
            ClearConnectionUsage({ conn, id });
        }
    }
    void ProfileManager::ClearConnectionUsage(const ProfileId &id)
    {
        Q_D(ProfileManager);
        CheckValidId(id.connectionId, nothing);
        d->connections[id.connectionId].statistics.clear();
        emit OnStatsAvailable(id, {});
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionStats>({ id.connectionId, {} });
        return;
    }
#endif
    const QList<GroupId> ProfileManager::GetGroups(const ConnectionId &connId) const
    {
        Q_D(const ProfileManager);
        CheckValidId(connId, {});
        QList<GroupId> grps;
        for (auto it = d->groups.constKeyValueBegin(); it != d->groups.constKeyValueEnd(); it++)
        {
            if (it->second.connections.contains(connId))
                grps.push_back(it->first);
        }
        return grps;
    }

    bool ProfileManager::RestartConnection()
    {
        return StartConnection(Qv2rayBaseLibrary::KernelManager()->CurrentConnection());
    }

    void ProfileManager::RenameConnection(const ConnectionId &id, const QString &newName)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        emit OnConnectionRenamed(id, d->connections[id].name, newName);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Renamed, NullGroupId, id, d->connections[id].name });
        d->connections[id].name = newName;
        SaveConnectionConfig();
    }

    bool ProfileManager::RemoveFromGroup(const ConnectionId &id, const GroupId &gid)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        QvLog() << "Removing connection:" << id;
        if (d->groups[gid].connections.contains(id))
        {
            auto removedEntries = d->groups[gid].connections.removeAll(id);
            if (removedEntries > 1)
            {
                QvLog() << "Found same connection occured multiple times in a group.";
            }
            // Decrease reference count.
            d->connections[id]._group_ref -= removedEntries;
        }

        // Emit everything first then clear the connection map.
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::RemovedFromGroup, gid, id, "" });
        emit OnConnectionRemovedFromGroup({ id, gid });

        if (d->connections[id]._group_ref <= 0)
        {
            QvLog() << "Fully removing a connection from cache.";
            d->connectionRootCache.remove(id);
            Qv2rayBaseLibrary::StorageProvider()->DeleteConnection(id);
            d->connections.remove(id);
        }
        return true;
    }

    bool ProfileManager::LinkWithGroup(const ConnectionId &id, const GroupId &newGroupId)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        if (d->groups[newGroupId].connections.contains(id))
        {
            QvLog() << "Connection not linked since" << id << "is already in the group" << newGroupId;
            return false;
        }
        d->groups[newGroupId].connections.append(id);
        d->connections[id]._group_ref++;
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::LinkedWithGroup, newGroupId, id, d->connections[id].name });
        emit OnConnectionLinkedWithGroup({ id, newGroupId });
        return true;
    }

    bool ProfileManager::MoveToGroup(const ConnectionId &id, const GroupId &sourceGid, const GroupId &targetGid)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        CheckValidId(targetGid, false);
        CheckValidId(sourceGid, false);

        if (!d->groups[sourceGid].connections.contains(id))
        {
            QvLog() << "Trying to move a connection away from a group it does not belong to.";
            return false;
        }
        if (d->groups[targetGid].connections.contains(id))
        {
            QvLog() << "The connection:" << id << "is already in the target group:" << targetGid;
            const auto removedCount = d->groups[sourceGid].connections.removeAll(id);
            d->connections[id]._group_ref -= removedCount;
        }
        else
        {
            // If the target group does not contain this connection.
            const auto removedCount = d->groups[sourceGid].connections.removeAll(id);
            d->connections[id]._group_ref -= removedCount;
            //
            d->groups[targetGid].connections.append(id);
            d->connections[id]._group_ref++;
        }

        emit OnConnectionRemovedFromGroup({ id, sourceGid });
        emit OnConnectionLinkedWithGroup({ id, targetGid });

        return true;
    }

    bool ProfileManager::DeleteGroup(const GroupId &id, bool removeConnections)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);

        if (id == DefaultGroupId)
        {
            if (removeConnections)
                for (const auto &conn : d->groups[id].connections)
                    RemoveFromGroup(conn, id);
            return false;
        }

        for (const auto &conn : d->groups[id].connections)
            if (removeConnections)
                RemoveFromGroup(conn, id);
            else
                MoveToGroup(conn, id, DefaultGroupId);

        const auto list = d->groups[id].connections;

        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::FullyRemoved, id, NullConnectionId, d->groups[id].name });
        d->groups.remove(id);
        SaveConnectionConfig();
        emit OnGroupDeleted(id, list);
        if (id == DefaultGroupId)
        {
            d->groups[id].name = tr("Default Group");
        }
        return {};
    }

    bool ProfileManager::StartConnection(const ProfileId &identifier)
    {
        Q_D(ProfileManager);
        CheckValidId(identifier, false);
        ProfileContent root = GetConnection(identifier.connectionId);
        auto errMsg = Qv2rayBaseLibrary::KernelManager()->StartConnection(identifier, root, d->routings.value(d->groups[identifier.groupId].route_id));
        if (errMsg)
        {
            Qv2rayBaseLibrary::Warn(tr("Failed to start connection"), *errMsg);
            return false;
        }
        d->connections[identifier.connectionId].last_connected = system_clock::now();
        return true;
    }

    void ProfileManager::StopConnection()
    {
        Qv2rayBaseLibrary::KernelManager()->StopConnection();
    }

    void ProfileManager::p_OnKernelCrashed(const ProfileId &id, const QString &errMessage)
    {
        QvLog() << "Kernel crashed:" << errMessage;
        emit OnKernelCrashed(id, errMessage);
    }

    ProfileManager::~ProfileManager()
    {
        SaveConnectionConfig();
    }

    const ProfileContent ProfileManager::GetConnection(const ConnectionId &id) const
    {
        Q_D(const ProfileManager);
        CheckValidId(id, ProfileContent());
        return d->connectionRootCache.value(id);
    }

#if QV2RAYBASE_FEATURE(latency)
    void ProfileManager::p_OnLatencyDataArrived(const ConnectionId &id, const Qv2rayPlugin::LatencyTestResponse &result)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        d->connections[id].latency = result.avg;
        emit OnLatencyTestFinished(id, result.avg);
    }
#endif

    void ProfileManager::UpdateConnection(const ConnectionId &id, const ProfileContent &root)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        d->connectionRootCache[id] = root;
        Qv2rayBaseLibrary::StorageProvider()->StoreConnection(id, root);
        emit OnConnectionModified(id);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Edited, NullGroupId, id, d->connections[id].name });
    }

    const GroupId ProfileManager::CreateGroup(const QString &displayName)
    {
        Q_D(ProfileManager);
        GroupId id(GenerateRandomString());
        d->groups[id].name = displayName;
        d->groups[id].created = system_clock::now();
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Created, id, NullConnectionId, displayName });
        emit OnGroupCreated(id, displayName);
        SaveConnectionConfig();
        return id;
    }

    const RoutingId ProfileManager::GetGroupRoutingId(const GroupId &id)
    {
        Q_D(ProfileManager);
        if (d->groups[id].route_id.isNull())
        {
            d->groups[id].route_id = RoutingId{ GenerateRandomString() };
        }
        return d->groups[id].route_id;
    }

    bool ProfileManager::RenameGroup(const GroupId &id, const QString &newName)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        emit OnGroupRenamed(id, d->groups[id].name, newName);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Renamed, id, NullConnectionId, d->groups[id].name });
        d->groups[id].name = newName;
        return true;
    }

#if QV2RAYBASE_FEATURE(subscriptions)
    void ProfileManager::SetSubscriptionData(const GroupId &id, const SubscriptionConfigObject &config)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        d->groups[id].subscription_config = config;
    }

    void ProfileManager::UpdateSubscriptionAsync(const GroupId &id)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        if (!d->groups[id].subscription_config.isSubscription)
            return;
        NetworkRequestHelper::AsyncHttpGet(d->groups[id].subscription_config.address, this, [this, id](const QByteArray &d) {
            p_CHUpdateSubscription(id, d);
            emit OnSubscriptionAsyncUpdateFinished(id);
        });
    }

    bool ProfileManager::UpdateSubscription(const GroupId &id)
    {
        Q_D(ProfileManager);
        if (!d->groups[id].subscription_config.isSubscription)
            return false;
        const auto data = NetworkRequestHelper::HttpGet(d->groups[id].subscription_config.address);
        return p_CHUpdateSubscription(id, data);
    }

    bool ProfileManager::p_CHUpdateSubscription(const GroupId &id, const QByteArray &data)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        //
        // ====================================================================================== Begin reading subscription
        std::shared_ptr<Qv2rayPlugin::Subscription::SubscriptionDecoder> decoder;
        {
            const auto type = d->groups[id].subscription_config.type;
            const auto sDecoder = Qv2rayBaseLibrary::PluginAPIHost()->Subscription_QueryType(type);

            if (!sDecoder)
            {
                Qv2rayBaseLibrary::Warn(tr("Cannot Update Subscription"),
                                        tr("Unknown subscription type: %1").arg(type) + NEWLINE + tr("A subscription plugin is missing?"));
                return false;
            }
            decoder = *sDecoder;
        }

        const auto result = decoder->DecodeData(data);

        QList<std::pair<QString, ProfileContent>> fetchedConnections;
        fetchedConnections.reserve(result.connections.size() + result.links.size());

        fetchedConnections << result.connections;

        for (const auto &link : result.links)
        {
            // Assign a group name, to pass the name check.
            const auto result = ConvertConfigFromString(link.trimmed());
            if (!result)
                QvLog() << "Error: Cannot decode share link: " << link;
            fetchedConnections << *result;
        }

        //
        // ====================================================================================== Begin Connection Data Storage
        // Anyway, we try our best to preserve the connection id.
        QMultiMap<QString, ConnectionId> nameMap;
        QMultiMap<std::tuple<QString, QString, int>, ConnectionId> typeMap;
        {
            // Store connection type metadata into map.
            for (const auto &conn : d->groups[id].connections)
            {
                nameMap.insert(GetDisplayName(conn), conn);
                const auto info = GetOutboundInfoTuple(GetConnection(conn).outbounds.first());
                typeMap.insert(info, conn);
            }
        }

        // ====================================================================================== End Connection Data Storage
        //
        bool hasErrorOccured = false;
        // Copy construct here.

        QList<ConnectionId> originalConnectionIdList;
        std::copy(d->groups[id].connections.constBegin(), d->groups[id].connections.constEnd(), originalConnectionIdList.begin());
        d->groups[id].connections.clear();

        QList<std::pair<QString, ProfileContent>> filteredConnections;
        for (const auto &config : fetchedConnections)
        {
            const auto includeRelation = d->groups[id].subscription_config.includeRelation;
            const auto excludeRelation = d->groups[id].subscription_config.excludeRelation;

            const auto includeKeywords = d->groups[id].subscription_config.includeKeywords;
            const auto excludeKeywords = d->groups[id].subscription_config.excludeKeywords;

            //
            // Matched cases (when ShouldHaveThisConnection is NOT ALTERED by the loop below):
            // Relation = OR  -> ShouldHaveThisConnection = FALSE --> None of the keywords can be found.       (Case 1: Since even one     match   will alter the value.)
            // Relation = AND -> ShouldHaveThisConnection = TRUE  --> All keywords are in the connection name. (Case 2: Since even one not matched will alter the value.)
            bool ShouldHaveThisConnection = includeRelation == SubscriptionConfigObject::RELATION_AND;
            {
                bool isIncludeKeywordsListEffective = false;
                for (const auto &includeKey : includeKeywords)
                {
                    // Empty, or spaced string is not "effective"
                    if (includeKey.trimmed().isEmpty())
                        continue;

                    isIncludeKeywordsListEffective = true;

                    // Matched cases:
                    // Case 1: Relation = OR  && Contains
                    // Case 2: Relation = AND && Not Contains
                    if ((includeRelation == SubscriptionConfigObject::RELATION_OR) == config.first.contains(includeKey.trimmed()))
                    {
                        // Relation = OR  -> Should     include the connection
                        // Relation = AND -> Should not include the connection (since keyword is "Not Contained" in the connection name).
                        ShouldHaveThisConnection = includeRelation == SubscriptionConfigObject::RELATION_OR;

                        // We have already made the decision, skip checking for other "include" keywords.
                        break;
                    }
                    else
                    {
                        // -- For other two cases:
                        // Case 3: Relation = OR  && Not Contains -> Check Next.
                        // Case 4: Relation = AND && Contains     -> Check Next: Cannot determine if all keywords are contained in the connection name.
                        continue;
                    }
                }

                // In case of the list of include keywords is empty: (We consider a QList<QString> with only empty strings, or just spaces, is still empty)
                if (!isIncludeKeywordsListEffective)
                    ShouldHaveThisConnection = true;
            }

            // Continue check for exclude relation if we still want this connection for now.
            if (ShouldHaveThisConnection)
            {
                bool isExcludeKeywordsListEffective = false;
                ShouldHaveThisConnection = excludeRelation == SubscriptionConfigObject::RELATION_OR;
                for (const auto &excludeKey : excludeKeywords)
                {
                    if (excludeKey.trimmed().isEmpty())
                        continue;

                    isExcludeKeywordsListEffective = true;

                    // Matched cases:
                    // Relation = OR  && Contains
                    // Relation = AND && Not Contains
                    if (excludeRelation == SubscriptionConfigObject::RELATION_OR == config.first.contains(excludeKey.trimmed()))
                    {
                        // Relation = OR  -> Should not include the connection (since the EXCLUDED keyword is in the connection name).
                        // Relation = AND -> Should     include the connection (since we can say "not ALL exclude keywords can be found", so that the AND relation breaks)
                        ShouldHaveThisConnection = excludeRelation == SubscriptionConfigObject::RELATION_AND;
                        break;
                    }
                    else
                    {
                        // -- For other two cases:
                        // Relation = OR  && Not Contains -> Check Next.
                        // Relation = AND && Contains     -> Check Next: Cannot determine if all keywords are contained in the connection name.
                        continue;
                    }
                }

                // See above comment for "isIncludeKeywordsListEffective"
                if (!isExcludeKeywordsListEffective)
                    ShouldHaveThisConnection = true;
            }

            // So we finally made the decision.
            if (ShouldHaveThisConnection)
                filteredConnections << config;
        }

        for (auto &[name, config] : filteredConnections)
        {
            // Should not have complex connection as we assume.
            const auto &&[protocol, host, port] = GetOutboundInfoTuple(config.outbounds.first());
            const auto outboundData = std::make_tuple(protocol, host, port);

            // Firstly we try to preserve connection ids by comparing with names.
            if (nameMap.contains(name))
            {
                // Just go and save the connection...
                QvLog() << "Reused connection id from name:" << name;
                const auto _conn = nameMap.take(name);
                d->groups[id].connections << _conn;
                UpdateConnection(_conn, config);
                // Remove Connection Id from the list.
                originalConnectionIdList.removeAll(_conn);
                typeMap.remove(typeMap.key(_conn));
                continue;
            }

            if (typeMap.contains(outboundData))
            {
                QvLog() << "Reused connection id from protocol/host/port pair for connection:" << name;
                const auto _conn = typeMap.take(outboundData);
                d->groups[id].connections << _conn;
                // Update Connection Properties
                UpdateConnection(_conn, config);
                RenameConnection(_conn, name);
                // Remove Connection Id from the list.
                originalConnectionIdList.removeAll(_conn);
                nameMap.remove(nameMap.key(_conn));
                continue;
            }

            // New connection id is required since nothing matched found...
            QvLog() << "Generated new connection id for connection:" << name;
            CreateConnection(config, name, id);
        }

        // In case there are deltas
        if (!originalConnectionIdList.isEmpty())
        {
            QvLog() << "Removed old d->connections not have been matched.";
            for (const auto &conn : originalConnectionIdList)
            {
                QvLog() << "Removing d->connections not in the new subscription:" << conn;
                RemoveFromGroup(conn, id);
            }
        }

        // Update the time
        d->groups[id].updated = system_clock::now();
        return hasErrorOccured;
    }

    void ProfileManager::IgnoreSubscriptionUpdate(const GroupId &group)
    {
        Q_D(ProfileManager);
        CheckValidId(group, nothing);
        if (d->groups[group].subscription_config.isSubscription)
            d->groups[group].updated = system_clock::now();
    }
#endif

#if QV2RAYBASE_FEATURE(statistics)
    void ProfileManager::p_OnStatsDataArrived(const ProfileId &id, const QMap<StatisticsObject::StatisticsType, StatisticsObject::StatsEntry> &speedData)
    {
        Q_D(ProfileManager);
        if (id.isNull())
            return;

        const auto &cid = id.connectionId;
        for (auto it = speedData.constKeyValueBegin(); it != speedData.constKeyValueEnd(); it++)
        {
            d->connections[cid].statistics[it->first].up += it->second.up;
            d->connections[cid].statistics[it->first].down += it->second.down;
        }

        emit OnStatsAvailable(id, speedData);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionStats>({ cid, d->connections[cid].statistics });
    }
#endif

    const ProfileId ProfileManager::CreateConnection(const ProfileContent &root, const QString &name, const GroupId &groupId)
    {
        Q_D(ProfileManager);
        QvLog() << "Creating new connection:" << name;
        ConnectionId newId(GenerateRandomString());
        d->groups[groupId].connections << newId;
        d->connections[newId].created = system_clock::now();
        d->connections[newId].name = name;
        d->connections[newId]._group_ref = 1;
        d->connectionRootCache[newId] = root;
        Qv2rayBaseLibrary::StorageProvider()->StoreConnection(newId, root);
        emit OnConnectionCreated({ newId, groupId }, name);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Created, groupId, newId, name });
        return { newId, groupId };
    }

    const QList<ConnectionId> ProfileManager::GetConnections() const
    {
        Q_D(const ProfileManager);
        return d->connections.keys();
    }

    const QList<ConnectionId> ProfileManager::GetConnections(const GroupId &groupId) const
    {
        Q_D(const ProfileManager);
        CheckValidId(groupId, {});
        return d->groups[groupId].connections;
    }

    const QList<GroupId> ProfileManager::GetGroups() const
    {
        Q_D(const ProfileManager);
        auto k = d->groups.keys();
        std::sort(k.begin(), k.end(), [&](const GroupId &idA, const GroupId &idB) { return d->groups[idA].name < d->groups[idB].name; });
        return k;
    }

    bool ProfileManager::IsValidId(const ConnectionId &id) const
    {
        Q_D(const ProfileManager);
        return d->connections.contains(id);
    }

    bool ProfileManager::IsValidId(const GroupId &id) const
    {
        Q_D(const ProfileManager);
        return d->groups.contains(id);
    }

    bool ProfileManager::IsValidId(const ProfileId &id) const
    {
        return IsValidId(id.connectionId) && IsValidId(id.groupId);
    }

    const ConnectionObject ProfileManager::GetConnectionObject(const ConnectionId &id) const
    {
        Q_D(const ProfileManager);
        CheckValidId(id, {});
        return d->connections[id];
    }

    const GroupObject ProfileManager::GetGroupObject(const GroupId &id) const
    {
        Q_D(const ProfileManager);
        CheckValidId(id, {});
        return d->groups[id];
    }

    bool ProfileManager::IsConnected(const ProfileId &id) const
    {
        return Qv2rayBaseLibrary::KernelManager()->CurrentConnection() == id;
    }

} // namespace Qv2rayBase::Profile

#undef CheckIdExistance
#undef CheckGroupExistanceEx
#undef CheckGroupExistance
#undef CheckConnectionExistanceEx
#undef CheckConnectionExistance
