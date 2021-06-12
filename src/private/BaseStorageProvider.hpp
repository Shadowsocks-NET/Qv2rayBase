#include "StorageProvider.hpp"

namespace Qv2rayBase::_private
{
    class Qv2rayBasePrivateStorageProvider : public IStorageProvider
    {
      public:
        virtual bool LookupConfigurations() override;
        virtual QJsonObject GetBaseConfiguration() override;
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
        virtual bool StoreBaseConfiguration(const QJsonObject &) override;
    };

} // namespace Qv2rayBase::_private
