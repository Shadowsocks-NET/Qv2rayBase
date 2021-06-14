#include "Common/ProfileHelpers.hpp"

#include "Common/Utils.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/ProfileManager.hpp"
#include "Qv2rayBaseLibrary.hpp"

#define QV_MODULE_NAME "CoreUtils"

namespace Qv2rayBase::Utils
{
#if QV2RAYBASE_FEATURE(latency)
    int64_t GetConnectionLatency(const ConnectionId &id)
    {
        const auto connection = Qv2rayBaseLibrary::ProfileManager()->GetConnectionObject(id);
        //        return std::max(*connection., {});
        return 0;
    }
#endif

    InboundObject GetInbound(const ConnectionId &id, int index)
    {
        return Qv2rayBaseLibrary::ProfileManager()->GetConnection(id).inbounds.at(index);
    }

    OutboundObject GetOutbound(const ConnectionId &id, int index)
    {
        return Qv2rayBaseLibrary::ProfileManager()->GetConnection(id).outbounds.at(index);
    }

    QString GetDisplayName(const ConnectionId &id)
    {
        return Qv2rayBaseLibrary::ProfileManager()->GetConnectionObject(id).name;
    }

    QString GetDisplayName(const GroupId &id)
    {
        return Qv2rayBaseLibrary::ProfileManager()->GetGroupObject(id).name;
    }

    std::tuple<QString, QString, int> GetOutboundInfoTuple(const OutboundObject &out)
    {
        const auto protocol = out.protocol;
        const auto info = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_GetData(out);
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
        for (const auto &item : out.outbounds)
        {
            result[item.name] = GetOutboundInfo(item);
        }
        return result;
    }

    std::tuple<QString, QString, int> GetInboundInfoTuple(const InboundObject &in)
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
        for (const auto &in : root.inbounds)
        {
            infomap[in["tag"].toString()] = GetInboundInfo(in);
        }
        return infomap;
    }

    PluginIOBoundData GetInboundInfo(const InboundObject &in)
    {
        return PluginIOBoundData{ { IOBOUND_DATA_TYPE::IO_PROTOCOL, in["protocol"].toString() },
                                  { IOBOUND_DATA_TYPE::IO_ADDRESS, in["listen"].toString() },
                                  { IOBOUND_DATA_TYPE::IO_PORT, in["port"].toInt() } };
    }

    PluginIOBoundData GetOutboundInfo(const OutboundObject &out)
    {
        const auto data = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_GetData(out);
        return data.value_or(decltype(data)::value_type());
    }

} // namespace Qv2rayBase::Utils
