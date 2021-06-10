#include "Common/ProfileHelper.hpp"
#include "Common/Utils.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/ProfileManager.hpp"
#include "Qv2rayBaseLibrary.hpp"

#define QV_MODULE_NAME "CoreUtils"

namespace Qv2rayBase::Utils
{
    using namespace Qv2rayPlugin;
    bool IsComplexConfig(const ProfileContent &root)
    {
        // If has routing, and there're rules exist.
        bool cRouting = root.contains("routing");
        bool cRule = cRouting && root["routing"].toObject().contains("rules");
        bool cRules = cRule && root["routing"].toObject()["rules"].toArray().count() > 0;

        // If has inbounds, and there're inbounds exist.
        bool cInbounds = root.contains("inbounds");
        bool cInboundCount = cInbounds && root["inbounds"].toArray().count() > 0;

        // If has outbounds, and there're more than 1 outbounds.
        bool cOutbounds = root.contains("outbounds");
        bool cOutboundCount = cOutbounds && root["outbounds"].toArray().count() > 1;
        return cRules || cInboundCount || cOutboundCount;
    }

    bool IsComplexConfig(const ConnectionId &id)
    {
        return IsComplexConfig(Qv2rayBaseLibrary::ProfileManager()->GetConnection(id));
    }

#if QV2RAYBASE_FEATURE(statistics)
    const std::tuple<quint64, quint64> GetConnectionUsageAmount(const ConnectionId &id)
    {
        auto connection = ConnectionManager->GetConnectionMetaObject(id);
        return { *connection.stats->get(CurrentStatAPIType).upLinkData, *connection.stats->get(CurrentStatAPIType).downLinkData };
    }

    uint64_t GetConnectionTotalData(const ConnectionId &id)
    {
        const auto &[a, b] = GetConnectionUsageAmount(id);
        return a + b;
    }
#endif

#if QV2RAYBASE_FEATURE(latency)
    int64_t GetConnectionLatency(const ConnectionId &id)
    {
        const auto connection = ConnectionManager->GetConnectionMetaObject(id);
        return std::max(*connection.latency, {});
    }
#endif

    const QString GetConnectionProtocolDescription(const ConnectionId &id)
    {
        // Don't bother with the complex connection configs.
        if (IsComplexConfig(id))
        {
            return "N/A";
        }

        const auto root = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        const auto outbound = root["outbounds"].toArray().first().toObject();

        QStringList result;
        result << outbound["protocol"].toString();

        const auto streamSettings = outbound["streamSettings"].toObject();

        if (streamSettings.contains("network"))
            result << streamSettings["network"].toString();

        const auto security = streamSettings["security"].toString();
        if (!security.isEmpty() && security != "none")
            result << streamSettings["security"].toString();

        return result.join("+");
    }

    QString GetDisplayName(const ConnectionId &id)
    {
        return Qv2rayBaseLibrary::ProfileManager()->GetConnectionObject(id).name;
    }

    QString GetDisplayName(const GroupId &id)
    {
        return Qv2rayBaseLibrary::ProfileManager()->GetGroupObject(id).name;
    }

    std::tuple<QString, QString, int> GetOutboundInfoTuple(const OUTBOUND &out)
    {
        const auto protocol = out["protocol"].toString(QObject::tr("N/A")).toLower();
        const auto info = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_GetData(protocol, out["settings"].toObject());
        if (info)
        {
            const auto val = *info;
            return { val[IOBOUND_DATA_TYPE::IO_PROTOCOL].toString(), val[IOBOUND_DATA_TYPE::IO_ADDRESS].toString(), val[IOBOUND_DATA_TYPE::IO_PORT].toInt() };
        }
        return { protocol, QObject::tr("N/A"), 0 };
    }

    QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ConnectionId &id)
    {
        const auto root = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        return GetOutboundsInfo(root);
    }

    QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ProfileContent &out)
    {
        QMap<QString, PluginIOBoundData> result;
        for (const auto &item : out["outbounds"].toArray())
        {
            const auto outboundRoot = OUTBOUND(item.toObject());
            result[getTag(outboundRoot)] = GetOutboundInfo(outboundRoot);
        }
        return result;
    }

    std::tuple<QString, QString, int> GetInboundInfoTuple(const INBOUND &in)
    {
        return { in["protocol"].toString(), in["listen"].toString(), in["port"].toVariant().toInt() };
    }

    QMap<QString, PluginIOBoundData> GetInboundsInfo(const ConnectionId &id)
    {
        return GetInboundsInfo(Qv2rayBaseLibrary::ProfileManager()->GetConnection(id));
    }

    QMap<QString, PluginIOBoundData> GetInboundsInfo(const ProfileContent &root)
    {
        QMap<QString, PluginIOBoundData> infomap;
        for (const auto &inRef : root["inbounds"].toArray())
        {
            const auto in = inRef.toObject();
            infomap[in["tag"].toString()] = GetInboundInfo(INBOUND(in));
        }
        return infomap;
    }

    PluginIOBoundData GetInboundInfo(const INBOUND &in)
    {
        return PluginIOBoundData{ { IOBOUND_DATA_TYPE::IO_PROTOCOL, in["protocol"].toString() },
                                  { IOBOUND_DATA_TYPE::IO_ADDRESS, in["listen"].toString() },
                                  { IOBOUND_DATA_TYPE::IO_PORT, in["port"].toInt() } };
    }

    PluginIOBoundData GetOutboundInfo(const OUTBOUND &out)
    {
        const auto data = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_GetData(out["protocol"].toString(), out["settings"].toObject());
        return data.value_or(decltype(data)::value_type());
    }

} // namespace Qv2rayBase::Utils
