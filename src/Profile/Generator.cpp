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

#include "Interfaces/IConfigurationGenerator.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/ProfileManager.hpp"

#define QV_MODULE_NAME "RouteHandler"

namespace Qv2rayBase::Interfaces
{
    bool IConfigurationGenerator::ExpandChains(ProfileContent &root)
    {
        // Proxy Chain Expansion
        const auto outbounds = root.outbounds;
        const auto inbounds = root.inbounds;
        const auto rules = root.routing.rules;

        QList<OutboundObject> finalOutbounds;
        QList<InboundObject> chainingInbounds;
        QList<RuleObject> chainingRules;

        // First pass - Resolve Indexes (tags), build cache
        QMap<QString, OutboundObject> outboundCache;
        for (const auto &outbound : outbounds)
        {
            if (outbound.objectType != OutboundObject::CHAIN)
                outboundCache[outbound.name] = outbound;
        }

        // Second pass - Build Chains
        for (const auto &outbound : outbounds)
        {
            if (outbound.objectType != OutboundObject::CHAIN)
            {
                finalOutbounds << outbound;
                continue;
            }

            if (outbound.chainSettings.chains.isEmpty())
            {
                QvLog() << "Trying to expand an empty chain.";
                continue;
            }

            int nextInboundPort = outbound.chainSettings.chaining_port;
            const auto firstOutboundTag = outbound.name;
            const auto lastOutboundTag = outbound.chainSettings.chains.first();

            PluginIOBoundData lastOutbound;

            const auto outbountTagCount = outbound.chainSettings.chains.count();

            for (auto i = outbountTagCount - 1; i >= 0; i--)
            {
                const auto chainOutboundTag = outbound.chainSettings.chains[i];

                const auto isFirstOutbound = i == outbountTagCount - 1;
                const auto isLastOutbound = i == 0;
                const auto newOutboundTag = [&]() {
                    if (isFirstOutbound)
                        return firstOutboundTag;
                    else if (isLastOutbound)
                        return lastOutboundTag;
                    else
                        return (firstOutboundTag + "_" + chainOutboundTag + "_" + QString::number(nextInboundPort));
                }();

                if (!outboundCache.contains(chainOutboundTag))
                {
                    QvLog() << "Cannot build outbound chain: Missing tag:" << firstOutboundTag;
                    return false;
                }

                auto newOutbound = outboundCache[chainOutboundTag];

                // Create Inbound
                if (!isFirstOutbound)
                {
                    const auto inboundTag = firstOutboundTag + ":" + QString::number(nextInboundPort) + "->" + newOutboundTag;

                    IOProtocolSettings inboundSettings;
                    inboundSettings[QStringLiteral("address")] = lastOutbound[IOBOUND_DATA_TYPE::IO_ADDRESS].toString();
                    inboundSettings[QStringLiteral("port")] = lastOutbound[IOBOUND_DATA_TYPE::IO_PORT].toInt();
                    inboundSettings[QStringLiteral("network")] = "tcp,udp";

                    InboundObject newInbound;
                    newInbound.name = inboundTag;
                    newInbound.inboundSettings.protocol = QStringLiteral("dokodemo-door");
                    newInbound.listenAddress = QStringLiteral("127.0.0.1");
                    newInbound.listenPort = QString::number(nextInboundPort);
                    newInbound.inboundSettings.protocolSettings = inboundSettings;

                    nextInboundPort++;
                    chainingInbounds << newInbound;
                    //
                    RuleObject ruleObject;
                    ruleObject.inboundTags.append(inboundTag);
                    ruleObject.outboundTag = newOutboundTag;
                    chainingRules.prepend(ruleObject);
                }

                if (!isLastOutbound)
                {
                    // Get Outbound Info for next Inbound
                    auto outboundSettings = newOutbound.outboundSettings;
                    const auto info = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_GetData(outboundSettings);
                    if (!info)
                    {
                        QvLog() << "Cannot get outbound info for:" << chainOutboundTag;
                        return false;
                    }
                    lastOutbound = *info;

                    // Update allocated port as outbound server/port
                    PluginIOBoundData newOutboundInfo;
                    newOutboundInfo.insert(IOBOUND_DATA_TYPE::IO_ADDRESS, "127.0.0.1");
                    newOutboundInfo.insert(IOBOUND_DATA_TYPE::IO_PORT, nextInboundPort);

                    // For those kernels deducing SNI from the server name.
                    if (!lastOutbound.contains(IOBOUND_DATA_TYPE::IO_SNI) || lastOutbound[IOBOUND_DATA_TYPE::IO_SNI].toString().trimmed().isEmpty())
                        newOutboundInfo.insert(IOBOUND_DATA_TYPE::IO_SNI, lastOutbound.value(IOBOUND_DATA_TYPE::IO_ADDRESS));

                    Qv2rayBaseLibrary::PluginAPIHost()->Outbound_SetData(outboundSettings, newOutboundInfo);
                    newOutbound.outboundSettings = outboundSettings;

                    // Create new outbound
                    newOutbound.name = newOutboundTag;
                }
                finalOutbounds << newOutbound;
            }
        }

        //
        // Finalize
        {
            root.inbounds.clear();
            root.inbounds << inbounds << chainingInbounds;

            root.routing.rules.clear();
            root.routing.rules << chainingRules << rules;

            root.outbounds = finalOutbounds;
        }
        return true;
    }

    QList<OutboundObject> IConfigurationGenerator::ExpandExternals(const QList<OutboundObject> &outbounds)
    {
        QList<OutboundObject> result;
        result.reserve(outbounds.size());
        for (auto out : outbounds)
        {
            if (out.objectType == OutboundObject::EXTERNAL)
            {
                const auto _outs = QvBaselib->ProfileManager()->GetConnection(out.externalId).outbounds;
                if (!_outs.isEmpty())
                {
                    auto newOut = _outs.first();
                    newOut.name = out.name;
                    out = newOut;
                }
            }
            result << out;
        }
        return result;
    }
} // namespace Qv2rayBase::Interfaces
