#pragma once
#include "Connections/ConnectionsBaseTypes.hpp"
#include "models/CoreObjectModels.hpp"
#include "models/QvSafeType.hpp"

#define CurrentStatAPIType (*GlobalConfig.uiConfig->graphConfig->useOutboundStats ? API_OUTBOUND_PROXY : API_INBOUND)

namespace Qv2ray::core
{
    using namespace Qv2ray::base;
    using namespace Qv2ray::base::safetype;
    using namespace Qv2ray::base::objects;

    inline const QString getTag(const INBOUND &in)
    {
        return in["tag"].toString();
    }

    inline const QString getTag(const OUTBOUND &in)
    {
        return in["tag"].toString();
    }

    inline const QString getTag(const RuleObject &in)
    {
        return in.QV2RAY_RULE_TAG;
    }

    template<typename R>
    R GetConnectionPart(const CONFIGROOT &r, int index)
    {
        if constexpr (std::is_same_v<R, INBOUND>)
            return INBOUND{ QJsonIO::GetValue(r, "inbounds", index).toObject() };
        if constexpr (std::is_same_v<R, OUTBOUND>)
            return OUTBOUND{ QJsonIO::GetValue(r, "outbounds", index).toObject() };
    }

    QV2RAYBASE_EXPORT bool IsComplexConfig(const CONFIGROOT &root);
    QV2RAYBASE_EXPORT bool IsComplexConfig(const ConnectionId &id);
    QV2RAYBASE_EXPORT QString GetDisplayName(const GroupId &id, int limit = -1);
    QV2RAYBASE_EXPORT QString GetDisplayName(const ConnectionId &id, int limit = -1);

    QV2RAYBASE_EXPORT const QString GetConnectionProtocolDescription(const ConnectionId &id);
    QV2RAYBASE_EXPORT int64_t GetConnectionLatency(const ConnectionId &id);
    QV2RAYBASE_EXPORT uint64_t GetConnectionTotalData(const ConnectionId &id);
    QV2RAYBASE_EXPORT const std::tuple<quint64, quint64> GetConnectionUsageAmount(const ConnectionId &id);

    QV2RAYBASE_EXPORT PluginIOBoundData GetInboundInfo(const INBOUND &in);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetInboundInfoTuple(const INBOUND &in);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetInboundsInfo(const CONFIGROOT &root);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetInboundsInfo(const ConnectionId &id);

    QV2RAYBASE_EXPORT PluginIOBoundData GetOutboundInfo(const OUTBOUND &out);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetOutboundInfoTuple(const OUTBOUND &out);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetOutboundsInfo(const CONFIGROOT &out);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ConnectionId &id);

} // namespace Qv2ray::core

using namespace Qv2ray::core;
