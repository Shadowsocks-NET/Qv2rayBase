#pragma once

#include "QvPluginInterface.hpp"

namespace Qv2rayBase::Profile
{
    class ProfileManagerPrivate
    {
      public:
        int pingAllTimerId;
        QHash<GroupId, GroupObject> groups;
        QHash<ConnectionId, ConnectionObject> connections;
        QHash<RoutingId, RoutingObject> routings;
        QHash<ConnectionId, ProfileContent> connectionRootCache;
    };
} // namespace Qv2rayBase::Profile
