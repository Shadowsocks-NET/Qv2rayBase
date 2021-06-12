#include "BaseStorageProvider.hpp"

namespace Qv2rayBase::_private
{
    bool Qv2rayBasePrivateStorageProvider::LookupConfigurations()
    {
        return false;
    }

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

    ProfileContent Qv2rayBasePrivateStorageProvider::GetConnectionContent(const ConnectionId &)
    {
        return {};
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

    QStringList Qv2rayBasePrivateStorageProvider::GetAssetsPath(const QString &)
    {
        return {};
    }

    bool Qv2rayBasePrivateStorageProvider::StoreBaseConfiguration(const QJsonObject &)
    {
        return {};
    }
} // namespace Qv2rayBase::_private
