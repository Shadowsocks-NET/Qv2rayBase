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
        return std::max(connection.latency, 0L);
    }

    std::pair<quint64, quint64> GetConnectionUsageAmount(const ConnectionId &id, StatisticsObject::StatisticsType type)
    {
        auto connection = Qv2rayBaseLibrary::ProfileManager()->GetConnectionObject(id);
        return { connection.statistics[type].up, connection.statistics[type].down };
    }

    quint64 GetConnectionTotalUsage(const ConnectionId &id, StatisticsObject::StatisticsType type)
    {
        const auto d = GetConnectionUsageAmount(id, type);
        return d.first + d.second;
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

    std::tuple<QString, QString, QString> GetInboundInfoTuple(const InboundObject &in)
    {
        return { in.protocol, in.listenAddress, in.listenPort };
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
            infomap[in.name] = GetInboundInfo(in);
        }
        return infomap;
    }

    PluginIOBoundData GetInboundInfo(const InboundObject &in)
    {
        return PluginIOBoundData{ { IOBOUND_DATA_TYPE::IO_PROTOCOL, in.protocol },
                                  { IOBOUND_DATA_TYPE::IO_ADDRESS, in.listenAddress },
                                  { IOBOUND_DATA_TYPE::IO_PORT, in.listenPort } };
    }

    PluginIOBoundData GetOutboundInfo(const OutboundObject &out)
    {
        const auto data = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_GetData(out);
        return data.value_or(decltype(data)::value_type());
    }

    QString GetConnectionProtocolDescription(const ConnectionId &id)
    {
        const auto root = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        const auto outbound = root.outbounds.first();

        QStringList result;
        result << outbound.protocol;

        const auto streamSettings = outbound.outboundSettings.streamSettings;

        if (streamSettings.contains("network"))
            result << streamSettings["network"].toString();

        const auto security = streamSettings["security"].toString();
        if (!security.isEmpty() && security != "none")
            result << streamSettings["security"].toString();

        return result.join("+");
    }

    std::optional<std::pair<QString, ProfileContent>> ConvertConfigFromString(const QString &link)
    {
        const auto optConf = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_Deserialize(link);
        if (!optConf)
            return std::nullopt;

        const auto &[name, outbound] = *optConf;
        return std::pair{ name, ProfileContent{ outbound } };
    }

    const QString ConvertConfigToString(const ConnectionId &id)
    {
        auto alias = GetDisplayName(id);
        auto server = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        return ConvertConfigToString(alias, server);
    }

    const QString ConvertConfigToString(const QString &alias, const ProfileContent &server)
    {
        const auto outbound = server.outbounds.first();
        return *Qv2rayBaseLibrary::PluginAPIHost()->Outbound_Serialize(alias, outbound);
    }

    bool IsComplexConfig(const ConnectionId &id)
    {
        const auto root = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        bool hasRouting = !root.routing.rules.isEmpty();
        bool hasInbound = !root.inbounds.isEmpty();
        bool hasAtLeastOneOutbounds = root.outbounds.size() > 1;
        return hasRouting || hasInbound || hasAtLeastOneOutbounds;
    }

} // namespace Qv2rayBase::Utils
