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

} // namespace Qv2rayBase::_private
