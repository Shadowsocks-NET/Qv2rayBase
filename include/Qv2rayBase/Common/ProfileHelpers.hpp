#pragma once
#include "Qv2rayBaseFeatures.hpp"
#include "QvPluginInterface.hpp"
#include "qv2raybase_export.h"

//#define CurrentStatAPIType (*GlobalConfig.uiConfig->graphConfig->useOutboundStats ? API_OUTBOUND_PROXY : API_INBOUND)

namespace Qv2rayBase::Utils
{
    template<typename R>
    R GetConnectionPart(const ProfileContent &r, int index)
    {
        if constexpr (std::is_same_v<R, InboundObject>)
            return r.inbounds.at(index);
        if constexpr (std::is_same_v<R, OutboundObject>)
            return r.outbounds.at(index);
    }

    QV2RAYBASE_EXPORT bool IsComplexConfig(const ProfileContent &root);
    QV2RAYBASE_EXPORT bool IsComplexConfig(const ConnectionId &id);
    QV2RAYBASE_EXPORT QString GetDisplayName(const GroupId &id);
    QV2RAYBASE_EXPORT QString GetDisplayName(const ConnectionId &id);

#if QV2RAYBASE_FEATURE(latency)
    QV2RAYBASE_EXPORT int64_t GetConnectionLatency(const ConnectionId &id);
#endif

#if QV2RAYBASE_FEATURE(statistics)
    QV2RAYBASE_EXPORT uint64_t GetConnectionTotalData(const ConnectionId &id);
    QV2RAYBASE_EXPORT const std::tuple<quint64, quint64> GetConnectionUsageAmount(const ConnectionId &id);
#endif

    QV2RAYBASE_EXPORT PluginIOBoundData GetInboundInfo(const InboundObject &in);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetInboundInfoTuple(const InboundObject &in);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetInboundsInfo(const ProfileContent &root);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetInboundsInfo(const ConnectionId &id);

    QV2RAYBASE_EXPORT PluginIOBoundData GetOutboundInfo(const OutboundObject &out);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetOutboundInfoTuple(const OutboundObject &out);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ProfileContent &out);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ConnectionId &id);
} // namespace Qv2rayBase::Utils

using namespace Qv2rayBase::Utils;
