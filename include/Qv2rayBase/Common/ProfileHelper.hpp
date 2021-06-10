#pragma once
#include "Qv2rayBaseFeatures.hpp"
#include "QvPluginInterface.hpp"
#include "qv2raybase_export.h"

//#define CurrentStatAPIType (*GlobalConfig.uiConfig->graphConfig->useOutboundStats ? API_OUTBOUND_PROXY : API_INBOUND)

namespace Qv2rayBase::Utils
{
    inline const QString getTag(const INBOUND &in)
    {
        return in["tag"].toString();
    }

    inline const QString getTag(const OUTBOUND &in)
    {
        return in["tag"].toString();
    }

    //    inline const QString getTag(const Qv2rayBase::models::RuleObject &in)
    //    {
    //        return in.QV2RAY_RULE_TAG;
    //    }

    template<typename R>
    R GetConnectionPart(const ProfileContent &r, int index)
    {
        if constexpr (std::is_same_v<R, INBOUND>)
            return INBOUND{ r["inbounds"].toArray().at(index).toObject() };
        if constexpr (std::is_same_v<R, OUTBOUND>)
            return OUTBOUND{ r["outbounds"].toArray().at(index).toObject() };
    }

    QV2RAYBASE_EXPORT bool IsComplexConfig(const ProfileContent &root);
    QV2RAYBASE_EXPORT bool IsComplexConfig(const ConnectionId &id);
    QV2RAYBASE_EXPORT QString GetDisplayName(const GroupId &id);
    QV2RAYBASE_EXPORT QString GetDisplayName(const ConnectionId &id);

    QV2RAYBASE_EXPORT const QString GetConnectionProtocolDescription(const ConnectionId &id);

#if QV2RAYBASE_FEATURE(latency)
    QV2RAYBASE_EXPORT int64_t GetConnectionLatency(const ConnectionId &id);
#endif

#if QV2RAYBASE_FEATURE(statistics)
    QV2RAYBASE_EXPORT uint64_t GetConnectionTotalData(const ConnectionId &id);
    QV2RAYBASE_EXPORT const std::tuple<quint64, quint64> GetConnectionUsageAmount(const ConnectionId &id);
#endif

    QV2RAYBASE_EXPORT Qv2rayPlugin::PluginIOBoundData GetInboundInfo(const INBOUND &in);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetInboundInfoTuple(const INBOUND &in);
    QV2RAYBASE_EXPORT QMap<QString, Qv2rayPlugin::PluginIOBoundData> GetInboundsInfo(const ProfileContent &root);
    QV2RAYBASE_EXPORT QMap<QString, Qv2rayPlugin::PluginIOBoundData> GetInboundsInfo(const ConnectionId &id);

    QV2RAYBASE_EXPORT Qv2rayPlugin::PluginIOBoundData GetOutboundInfo(const OUTBOUND &out);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetOutboundInfoTuple(const OUTBOUND &out);
    QV2RAYBASE_EXPORT QMap<QString, Qv2rayPlugin::PluginIOBoundData> GetOutboundsInfo(const ProfileContent &out);
    QV2RAYBASE_EXPORT QMap<QString, Qv2rayPlugin::PluginIOBoundData> GetOutboundsInfo(const ConnectionId &id);
} // namespace Qv2rayBase::Utils

using namespace Qv2rayBase::Utils;
