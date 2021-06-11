#include "StorageProvider.hpp"

namespace Qv2rayBase::_private
{
    class Qv2rayBasePrivateStorageProvider : public IStorageProvider
    {
      public:
        virtual QJsonObject GetBaseConfiguration() override;
        virtual QHash<ConnectionId, ConnectionObject> Connections() override;
        virtual QHash<GroupId, GroupObject> Groups() override;
        virtual QHash<RoutingId, RoutingObject> Routings() override;
        virtual ProfileContent LoadConnection(const ConnectionId &) override;
        virtual bool StoreConnection(const ConnectionId &, ProfileContent) override;
        virtual bool DeleteConnection(const ConnectionId &id) override;
        virtual QDir GetPluginWorkingDirectory(const PluginId &pid) override;
        virtual QJsonObject GetPluginSettings(const PluginId &pid) override;
        virtual void SetPluginSettings(const PluginId &pid, const QJsonObject &obj) override;
    };

} // namespace Qv2rayBase::_private
