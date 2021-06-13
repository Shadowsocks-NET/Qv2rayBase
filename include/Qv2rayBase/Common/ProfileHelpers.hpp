#pragma once
#include "Qv2rayBaseFeatures.hpp"
#include "Qv2rayBase_export.h"
#include "QvPluginInterface.hpp"

namespace Qv2rayBase::Utils
{
    QV2RAYBASE_EXPORT InboundObject GetInbound(const ConnectionId &id, int index);
    QV2RAYBASE_EXPORT OutboundObject GetOutbound(const ConnectionId &id, int index);
    QV2RAYBASE_EXPORT QString GetDisplayName(const GroupId &id);
    QV2RAYBASE_EXPORT QString GetDisplayName(const ConnectionId &id);

#if QV2RAYBASE_FEATURE(latency)
    QV2RAYBASE_EXPORT int64_t GetConnectionLatency(const ConnectionId &id);
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
