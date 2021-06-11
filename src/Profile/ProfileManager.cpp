#include "Profile/ProfileManager.hpp"

#include "Common/Utils.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/Generator.hpp"
#include "Profile/KernelManager.hpp"
#include "Profile/Serialization.hpp"
#include "StorageProvider.hpp"
#include "src/Utils/HTTPRequestHelper.hpp"

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
    class ProfileManagerPrivate
    {
      public:
        int pingAllTimerId;
        QHash<GroupId, GroupObject> groups;
        QHash<ConnectionId, ConnectionObject> connections;
        QHash<RoutingId, RoutingObject> routings;
        QHash<ConnectionId, ProfileContent> connectionRootCache;
    };

    ProfileManager::ProfileManager(QObject *parent) : QObject(parent)
    {
        Q_D(ProfileManager)
        DEBUG("ConnectionHandler Constructor.");

        const auto _connections = Qv2rayBaseLibrary::StorageProvider()->Connections();
        const auto _groups = Qv2rayBaseLibrary::StorageProvider()->Groups();
        const auto _routings = Qv2rayBaseLibrary::StorageProvider()->Routings();

        for (auto it = _connections.constKeyValueBegin(); it != _connections.constKeyValueEnd(); it++)
        {
            d->connections.insert(it->first, it->second);
        }

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

        for (auto it = _connections.constKeyValueBegin(); it != _connections.constKeyValueEnd(); it++)
        {
            auto id = it->first;
            auto conn = it->second;
            auto const &connectionObject = d->connections[id];
            if (connectionObject._group_ref == 0)
            {
                d->connections.remove(id);
                Qv2rayBaseLibrary::StorageProvider()->DeleteConnection(id);
                LOG("Dropped connection id: " + id.toString() + " since it's not in a group");
            }
            else
            {
                d->connectionRootCache[id] = Qv2rayBaseLibrary::StorageProvider()->LoadConnection(id);
                DEBUG("Loaded connection id: " + id.toString() + " into cache.");
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
    }

    //    void ProfileManager::timerEvent(QTimerEvent *event)
    //    {
    //        Q_D(ProfileManager);
    //        if (event->timerId() == d->saveTimerId)
    //        {
    //            SaveConnectionConfig();
    //        }
    //        else if (event->timerId() == d->pingAllTimerId)
    //        {
    //            StartLatencyTest();
    //        }
    //    }

    //    void ProfileManager::StartLatencyTest()
    //    {
    //        for (const auto &connection : d->connections.keys())
    //        {
    //            emit OnLatencyTestStarted(connection);
    //        }
    //#pragma message("TODO")
    //        //        pingHelper->TestLatency(connections.keys(), GlobalConfig.networkConfig->latencyTestingMethod);
    //    }

    //    void ProfileManager::StartLatencyTest(const GroupId &id)
    //    {
    //        for (const auto &connection : *groups[id].connections)
    //        {
    //            emit OnLatencyTestStarted(connection);
    //        }
    //#pragma message("TODO")
    //        //        pingHelper->TestLatency(groups[id].connections, GlobalConfig.networkConfig->latencyTestingMethod);
    //    }

    //    void ProfileManager::StartLatencyTest(const ConnectionId &id, Qv2rayLatencyTestingMethod method)
    //    {
    //        emit OnLatencyTestStarted(id);
    //#pragma message("TODO")
    //        //        pingHelper->TestLatency(id, method);
    //    }

    //    const QList<GroupId> ProfileManager::Subscriptions() const
    //    {
    //        QList<GroupId> subsList;

    //        for (const auto &group : groups)
    //        {
    //            if (group.isSubscription)
    //            {
    //                subsList.push_back(groups.key(group));
    //            }
    //        }

    //        return subsList;
    //    }

    //    void ProfileManager::ClearGroupUsage(const GroupId &id)
    //    {
    //        for (const auto &conn : *groups[id].connections)
    //        {
    //            ClearConnectionUsage({ conn, id });
    //        }
    //    }
    //    void ProfileManager::ClearConnectionUsage(const ConnectionGroupPair &id)
    //    {
    //        CheckValidId(id.connectionId, nothing);
    //        d->connections[id.connectionId].stats->Clear();
    //        emit OnStatsAvailable(id, {});
    //        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionStats>({ GetDisplayName(id.connectionId), 0, 0, 0, 0 });
    //        return;
    //    }

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

    void ProfileManager::RenameConnection(const ConnectionId &id, const QString &newName)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        emit OnConnectionRenamed(id, d->connections[id].name, newName);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Renamed, newName, d->connections[id].name });
        d->connections[id].name = newName;
        SaveConnectionConfig();
    }

    bool ProfileManager::RemoveFromGroup(const ConnectionId &id, const GroupId &gid)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        LOG("Removing connection : " + id.toString());
        if (d->groups[gid].connections.contains(id))
        {
            auto removedEntries = d->groups[gid].connections.removeAll(id);
            if (removedEntries > 1)
            {
                LOG("Found same connection occured multiple times in a group.");
            }
            // Decrease reference count.
            d->connections[id]._group_ref -= removedEntries;
        }

        // Emit everything first then clear the connection map.
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::RemovedFromGroup, GetDisplayName(id), "" });
        emit OnConnectionRemovedFromGroup({ id, gid });

        if (d->connections[id]._group_ref <= 0)
        {
            LOG("Fully removing a connection from cache.");
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
            LOG("Connection not linked since " + id.toString() + " is already in the group " + newGroupId.toString());
            return false;
        }
        d->groups[newGroupId].connections.append(id);
        d->connections[id]._group_ref++;
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::LinkedWithGroup, d->connections[id].name, "" });
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
            LOG("Trying to move a connection away from a group it does not belong to.");
            return false;
        }
        if (d->groups[targetGid].connections.contains(id))
        {
            LOG("The connection: " + id.toString() + " has already been in the target group: " + targetGid.toString());
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

    const std::optional<QString> ProfileManager::DeleteGroup(const GroupId &id)
    {
        Q_D(ProfileManager);
        CheckValidId(id, tr("Group does not exist"));
        // Copy construct
        auto list = d->groups[id].connections;
        for (const auto &conn : list)
        {
            MoveToGroup(conn, id, DefaultGroupId);
        }

        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::FullyRemoved, d->groups[id].name, "" });
        d->groups.remove(id);
        SaveConnectionConfig();
        emit OnGroupDeleted(id, list);
        if (id == DefaultGroupId)
        {
            d->groups[id].name = tr("Default Group");
        }
        return {};
    }

    bool ProfileManager::StartConnection(const ConnectionGroupPair &identifier)
    {
        Q_D(ProfileManager);
        CheckValidId(identifier, false);
        d->connections[identifier.connectionId].last_connected = system_clock::now();
        ProfileContent root = GetConnection(identifier.connectionId);
        const auto fullConfig = Qv2rayBaseLibrary::ConfigurationGenerator()->ApplyRouting(root, d->routings.value(d->groups[identifier.groupId].route_id));
        auto errMsg = Qv2rayBaseLibrary::KernelManager()->StartConnection(identifier, fullConfig);
        if (errMsg)
        {
            Qv2rayBaseLibrary::Warn(tr("Failed to start connection"), *errMsg);
            return false;
        }
        return true;
    }

    void ProfileManager::StopConnection()
    {
        Qv2rayBaseLibrary::KernelManager()->StopConnection();
    }

    void ProfileManager::p_OnKernelCrashed(const ConnectionGroupPair &id, const QString &errMessage)
    {
        LOG("Kernel crashed: " + errMessage);
        emit OnKernelCrashed(id, errMessage);
    }

    ProfileManager::~ProfileManager()
    {
        LOG("Triggering save settings from destructor");
        SaveConnectionConfig();
    }

    const ProfileContent ProfileManager::GetConnection(const ConnectionId &id) const
    {
        Q_D(const ProfileManager);
        CheckValidId(id, ProfileContent());
        return d->connectionRootCache.value(id);
    }

#if QV2RAYBASE_FEATURE(latency)
    void ProfileManager::p_OnLatencyDataArrived(const ConnectionId &id, const LatencyTestResult &result)
    {
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
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Edited, d->connections[id].name, "" });
    }

    const GroupId ProfileManager::CreateGroup(const QString &displayName)
    {
        Q_D(ProfileManager);
        GroupId id(GenerateRandomString());
        d->groups[id].name = displayName;
        d->groups[id].created = system_clock::now();
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Created, displayName, "" });
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

    const std::optional<QString> ProfileManager::RenameGroup(const GroupId &id, const QString &newName)
    {
        Q_D(ProfileManager);
        CheckValidId(id, tr("Group does not exist"));
        emit OnGroupRenamed(id, d->groups[id].name, newName);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Renamed, newName, d->groups[id].name });
        d->groups[id].name = newName;
        return {};
    }

#if QV2RAYBASE_FEATURE(subscriptions)
    bool ProfileManager::SetSubscriptionData(const GroupId &id, std::optional<bool> isSubscription, const std::optional<QString> &address,
                                             std::optional<float> updateInterval)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);

        if (isSubscription.has_value())
            groups[id].isSubscription = *isSubscription;

        if (address.has_value())
            groups[id].subscriptionOption->address = *address;

        if (updateInterval.has_value())
            groups[id].subscriptionOption->updateInterval = *updateInterval;

        return true;
    }

    bool ProfileManager::SetSubscriptionType(const GroupId &id, const QString &type)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        groups[id].subscriptionOption->type = type;
        return true;
    }

    bool ProfileManager::SetSubscriptionIncludeKeywords(const GroupId &id, const QStringList &keywords)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        groups[id].subscriptionOption->includeKeywords->clear();

        for (const auto &keyword : keywords)
        {
            if (!keyword.trimmed().isEmpty())
            {
                groups[id].subscriptionOption->includeKeywords->push_back(keyword);
            }
        }
        return true;
    }

    bool ProfileManager::SetSubscriptionIncludeRelation(const GroupId &id, SubscriptionFilterRelation relation)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        groups[id].subscriptionOption->includeRelation = relation;
        return true;
    }

    bool ProfileManager::SetSubscriptionExcludeKeywords(const GroupId &id, const QStringList &keywords)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        groups[id].subscriptionOption->excludeKeywords->clear();
        for (const auto &keyword : keywords)
        {
            if (!keyword.trimmed().isEmpty())
            {
                groups[id].subscriptionOption->excludeKeywords->push_back(keyword);
            }
        }
        return true;
    }

    bool ProfileManager::SetSubscriptionExcludeRelation(const GroupId &id, SubscriptionFilterRelation relation)
    {
        Q_D(ProfileManager);
        CheckValidId(id, false);
        groups[id].subscriptionOption->excludeRelation = relation;
        return true;
    }

    void ProfileManager::UpdateSubscriptionAsync(const GroupId &id)
    {
        Q_D(ProfileManager);
        CheckValidId(id, nothing);
        if (!groups[id].isSubscription)
            return;
        NetworkRequestHelper::AsyncHttpGet(groups[id].subscriptionOption->address, [=](const QByteArray &d) {
            p_CHUpdateSubscription(id, d);
            emit OnSubscriptionAsyncUpdateFinished(id);
        });
    }

    bool ProfileManager::UpdateSubscription(const GroupId &id)
    {
        if (!groups[id].isSubscription)
            return false;
        const auto data = NetworkRequestHelper::HttpGet(*groups[id].subscriptionOption->address);
        return p_CHUpdateSubscription(id, data);
    }

    bool ProfileManager::p_CHUpdateSubscription(const GroupId &id, const QByteArray &data)
    {
        CheckValidId(id, false);
        //
        // ====================================================================================== Begin reading subscription
        std::shared_ptr<SubscriptionDecoder> decoder;

        {
            const auto type = *groups[id].subscriptionOption->type;
            const auto sDecoder = Qv2rayBaseLibrary::PluginAPIHost()->Subscription_QueryType(type);

            if (!sDecoder)
            {
                QvMessageBoxWarn(nullptr, tr("Cannot Update Subscription"),
                                 tr("Unknown subscription type: %1").arg(type) + NEWLINE + tr("A subscription plugin is missing?"));
                return false;
            }
            decoder = *sDecoder;
        }

        const auto groupName = *groups[id].name;
        const auto result = decoder->DecodeData(data);
        QList<std::pair<QString, ProfileContent>> _newConnections;

        for (const auto &[name, json] : result.connections)
        {
            _newConnections.append({ name, ProfileContent(json) });
        }
        for (const auto &link : result.links)
        {
            // Assign a group name, to pass the name check.
            QString _alias;
            QString errMessage;
            QString __groupName = groupName;
            const auto connectionConfigMap = ConvertConfigFromString(link.trimmed(), &_alias, &errMessage, &__groupName);
            if (!errMessage.isEmpty())
                LOG("Error: ", errMessage);
            _newConnections << connectionConfigMap;
        }

        if (_newConnections.count() < 5)
        {
            LOG("Found a subscription with less than 5 d->connections.");
            if (QvMessageBoxAsk(nullptr, tr("Update Subscription"),
                                tr("%n entrie(s) have been found from the subscription source, do you want to continue?", "", _newConnections.count())) != Yes)
                return false;
        }
        //
        // ====================================================================================== Begin Connection Data Storage
        // Anyway, we try our best to preserve the connection id.
        QMultiMap<QString, ConnectionId> nameMap;
        QMultiMap<std::tuple<QString, QString, int>, ConnectionId> typeMap;
        {
            // Store connection type metadata into map.
            for (const auto &conn : *groups[id].connections)
            {
                nameMap.insert(GetDisplayName(conn), conn);
                const auto &&[protocol, host, port] = GetOutboundInfoTuple(OUTBOUND{ connectionRootCache[conn]["outbounds"].toArray()[0].toObject() });
                if (port != 0)
                {
                    typeMap.insert({ protocol, host, port }, conn);
                }
            }
        }
        // ====================================================================================== End Connection Data Storage
        //
        bool hasErrorOccured = false;
        // Copy construct here.
        auto originalConnectionIdList = *groups[id].connections;
        groups[id].connections->clear();
        //
        decltype(_newConnections) filteredConnections;
        //
        for (const auto &config : _newConnections)
        {
            // filter d->connections
            const bool isIncludeOperationAND = groups[id].subscriptionOption->includeRelation == RELATION_AND;
            const bool isExcludeOperationOR = groups[id].subscriptionOption->excludeRelation == RELATION_OR;
            //
            // Initial includeConfig value
            bool includeconfig = isIncludeOperationAND;
            {
                bool hasIncludeItemMatched = false;
                for (const auto &key : *groups[id].subscriptionOption->includeKeywords)
                {
                    if (!key.trimmed().isEmpty())
                    {
                        hasIncludeItemMatched = true;
                        // WARN: MAGIC, DO NOT TOUCH
                        if (!isIncludeOperationAND == config.first.contains(key.trimmed()))
                        {
                            includeconfig = !isIncludeOperationAND;
                            break;
                        }
                    }
                }
                // If includekeywords is empty then include all configs.
                if (!hasIncludeItemMatched)
                    includeconfig = true;
            }
            if (includeconfig)
            {
                bool hasExcludeItemMatched = false;
                includeconfig = isExcludeOperationOR;
                for (const auto &key : *groups[id].subscriptionOption->excludeKeywords)
                {
                    if (!key.trimmed().isEmpty())
                    {
                        hasExcludeItemMatched = true;
                        // WARN: MAGIC, DO NOT TOUCH
                        if (isExcludeOperationOR == config.first.contains(key.trimmed()))
                        {
                            includeconfig = !isExcludeOperationOR;
                            break;
                        }
                    }
                }
                // If excludekeywords is empty then don't exclude any configs.
                if (!hasExcludeItemMatched)
                    includeconfig = true;
            }

            if (includeconfig)
            {
                filteredConnections << config;
            }
        }

        LOG("Filtered out less than 5 d->connections.");
        const auto useFilteredConnections =
            filteredConnections.count() > 5 ||
            QvMessageBoxAsk(nullptr, tr("Update Subscription"),
                            tr("%1 out of %n entrie(s) have been filtered out, do you want to continue?", "", _newConnections.count()).arg(filteredConnections.count()) +
                                NEWLINE + GetDisplayName(id)) == Yes;

        for (const auto &config : useFilteredConnections ? filteredConnections : _newConnections)
        {
            const auto &_alias = config.first;
            // Should not have complex connection we assume.
            const auto &&[protocol, host, port] = GetOutboundInfoTuple(OUTBOUND{ config.second["outbounds"].toArray()[0].toObject() });
            const auto outboundData = std::make_tuple(protocol, host, port);
            //
            // ====================================================================================== Begin guessing new ConnectionId
            if (nameMap.contains(_alias))
            {
                // Just go and save the connection...
                LOG("Reused connection id from name: " + _alias);
                const auto _conn = nameMap.take(_alias);
                groups[id].connections << _conn;
                UpdateConnection(_conn, config.second, true);
                // Remove Connection Id from the list.
                originalConnectionIdList.removeAll(_conn);
                typeMap.remove(typeMap.key(_conn));
            }
            else if (typeMap.contains(outboundData))
            {
                LOG("Reused connection id from protocol/host/port pair for connection: " + _alias);
                const auto _conn = typeMap.take(outboundData);
                groups[id].connections << _conn;
                // Update Connection Properties
                UpdateConnection(_conn, config.second, true);
                RenameConnection(_conn, _alias);
                // Remove Connection Id from the list.
                originalConnectionIdList.removeAll(_conn);
                nameMap.remove(nameMap.key(_conn));
            }
            else
            {
                // New connection id is required since nothing matched found...
                LOG("Generated new connection id for connection: " + _alias);
                CreateConnection(config.second, _alias, id, true);
            }
            // ====================================================================================== End guessing new ConnectionId
        }

        // Check if anything left behind (not being updated or changed significantly)
        if (!originalConnectionIdList.isEmpty())
        {
            bool needContinue = QvMessageBoxAsk(nullptr, //
                                                tr("Update Subscription"),
                                                tr("There're %n connection(s) in the group that do not belong the current subscription (any more).", "",
                                                   originalConnectionIdList.count()) +
                                                    NEWLINE + GetDisplayName(id) + NEWLINE + tr("Would you like to remove them?")) == Yes;
            if (needContinue)
            {
                LOG("Removed old d->connections not have been matched.");
                for (const auto &conn : originalConnectionIdList)
                {
                    LOG("Removing d->connections not in the new subscription: " + conn.toString());
                    RemoveFromGroup(conn, id);
                }
            }
        }

        // Update the time
        groups[id].lastUpdatedDate = system_clock::to_time_t(system_clock::now());
        return hasErrorOccured;
    }

    void ProfileManager::IgnoreSubscriptionUpdate(const GroupId &group)
    {
        Q_D(const ProfileManager);
        CheckValidId(group, nothing);
        if (groups[group].isSubscription)
            groups[group].lastUpdatedDate = system_clock::to_time_t(system_clock::now());
    }
#endif

#if QV2RAYBASE_FEATURE(statistics)
    void ProfileManager::p_OnStatsDataArrived(const ConnectionGroupPair &id, const QMap<StatisticsType, QvStatsSpeed> &data)
    {
        Q_D(ProfileManager);
        if (id.isEmpty())
            return;

        const auto &cid = id.connectionId;
        QMap<StatisticsType, QvStatsSpeedData> result;
        for (const auto t : data.keys())
        {
            const auto &stat = data[t];
            d->connections[cid].stats->get(t).upLinkData += stat.first;
            d->connections[cid].stats->get(t).downLinkData += stat.second;
            result[t] = { stat, d->connections[cid].stats->get(t).toData() };
        }

        emit OnStatsAvailable(id, result);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionStats>({ GetDisplayName(cid),                     //
                                                                          result[CurrentStatAPIType].first.first,  //
                                                                          result[CurrentStatAPIType].first.second, //
                                                                          result[CurrentStatAPIType].second.first, //
                                                                          result[CurrentStatAPIType].second.second });
    }
#endif

    const ConnectionGroupPair ProfileManager::CreateConnection(const ProfileContent &root, const QString &displayName, const GroupId &groupId)
    {
        Q_D(ProfileManager);
        LOG("Creating new connection: " + displayName);
        ConnectionId newId(GenerateRandomString());
        d->groups[groupId].connections << newId;
        d->connections[newId].created = system_clock::now();
        d->connections[newId].name = displayName;
        d->connections[newId]._group_ref = 1;
        emit OnConnectionCreated({ newId, groupId }, displayName);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<ConnectionEntry>({ ConnectionEntry::Created, displayName, "" });
        UpdateConnection(newId, root);
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

    bool ProfileManager::IsValidId(const ConnectionGroupPair &id) const
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

    bool ProfileManager::IsConnected(const ConnectionGroupPair &id) const
    {
        return Qv2rayBaseLibrary::KernelManager()->CurrentConnection() == id;
    }

} // namespace Qv2rayBase::Profile

#undef CheckIdExistance
#undef CheckGroupExistanceEx
#undef CheckGroupExistance
#undef CheckConnectionExistanceEx
#undef CheckConnectionExistance
