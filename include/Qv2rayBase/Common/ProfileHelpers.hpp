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

    QV2RAYBASE_EXPORT QString GetConnectionProtocolDescription(const ConnectionId &id);
    QV2RAYBASE_EXPORT std::optional<std::pair<QString, ProfileContent>> ConvertConfigFromString(const QString &link);
    QV2RAYBASE_EXPORT std::optional<QString> ConvertConfigToString(const ConnectionId &id);
    QV2RAYBASE_EXPORT std::optional<QString> ConvertConfigToString(const QString &alias, const ProfileContent &root);
    QV2RAYBASE_EXPORT bool IsComplexConfig(const ConnectionId &id);

#if QV2RAYBASE_FEATURE(latency)
    QV2RAYBASE_EXPORT int64_t GetConnectionLatency(const ConnectionId &id);
    QV2RAYBASE_EXPORT std::pair<quint64, quint64> GetConnectionUsageAmount(const ConnectionId &id, StatisticsObject::StatisticsType type);
    QV2RAYBASE_EXPORT quint64 GetConnectionTotalUsage(const ConnectionId &id, StatisticsObject::StatisticsType type);
#endif

    QV2RAYBASE_EXPORT PluginIOBoundData GetInboundInfo(const InboundObject &in);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, QString> GetInboundInfoTuple(const InboundObject &in);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetInboundsInfo(const ProfileContent &root);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetInboundsInfo(const ConnectionId &id);

    QV2RAYBASE_EXPORT PluginIOBoundData GetOutboundInfo(const OutboundObject &out);
    QV2RAYBASE_EXPORT std::tuple<QString, QString, int> GetOutboundInfoTuple(const OutboundObject &out);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ProfileContent &out);
    QV2RAYBASE_EXPORT QMap<QString, PluginIOBoundData> GetOutboundsInfo(const ConnectionId &id);
} // namespace Qv2rayBase::Utils

using namespace Qv2rayBase::Utils;
