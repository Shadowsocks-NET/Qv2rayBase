#include "BaseStorageProvider.hpp"

namespace Qv2rayBase::_private
{
    QJsonObject Qv2rayBasePrivateStorageProvider::GetBaseConfiguration()
    {
        return {};
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

    ProfileContent Qv2rayBasePrivateStorageProvider::LoadConnection(const ConnectionId &)
    {
        ProfileContent p;
        return p;
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
        return;
    }
} // namespace Qv2rayBase::_private
