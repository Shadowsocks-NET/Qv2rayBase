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

#include "Plugin/PluginAPIHost.hpp"

#include "Common/Utils.hpp"
#include "Plugin/PluginManagerCore.hpp"
#include "Qv2rayBaseLibrary.hpp"
#include "private/Plugin/PluginAPIHost_p.hpp"
#include "private/Plugin/PluginManagerCore_p.hpp"

#define QV_MODULE_NAME "PluginAPIHost"

using namespace Qv2rayPlugin;

namespace Qv2rayBase::Plugin
{
    using namespace Qv2rayPlugin::Event;
    using namespace Qv2rayPlugin::Kernel;
    using namespace Qv2rayPlugin::Outbound;
    using namespace Qv2rayPlugin::Subscription;

    PluginAPIHost::PluginAPIHost()
    {
    }

    PluginAPIHost::~PluginAPIHost()
    {
    }

    LatencyTestEngineInfo PluginAPIHost::Latency_GetEngine(const LatencyTestEngineId &id) const
    {
        Q_D(const PluginAPIHost);
        return d->latencyTesters[id];
    }

    KernelFactory PluginAPIHost::Kernel_GetInfo(const KernelId &kid) const
    {
        Q_D(const PluginAPIHost);
        return d->kernels[kid];
    }

    KernelId PluginAPIHost::Kernel_GetDefaultKernel() const
    {
        Q_D(const PluginAPIHost);
        qsizetype supportedProtocolsCount = 0;
        KernelId result;
        for (auto it = d->kernels.constKeyValueBegin(); it != d->kernels.constKeyValueEnd(); it++)
        {
            if (it->second.Capabilities & KERNELCAP_ROUTER)
                if (it->second.SupportedProtocols.size() > supportedProtocolsCount)
                    result = it->first, supportedProtocolsCount = it->second.SupportedProtocols.size();
        }
        return result;
    }

    KernelId PluginAPIHost::Kernel_QueryProtocol(const QSet<QString> &protocols) const
    {
        Q_D(const PluginAPIHost);
        const KernelFactory *bestMatch = nullptr;
        qsizetype maxIntersections = 0;
        for (const auto &k : d->kernels)
        {
            const auto intersection = (k.SupportedProtocols & protocols).size();
            if (maxIntersections < intersection)
            {
                maxIntersections = intersection;
                bestMatch = &k;
            }
        }
        return bestMatch ? bestMatch->Id : NullKernelId;
    }

    std::optional<PluginIOBoundData> PluginAPIHost::Outbound_GetData(const OutboundObject &o) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            if (serializer && serializer->SupportedProtocols().contains(o.protocol))
            {
                auto info = serializer->GetOutboundInfo(o.protocol, o.outboundSettings);
                if (info)
                    return info;
            }
        }
        return std::nullopt;
    }

    bool PluginAPIHost::Outbound_SetData(OutboundObject &o, const PluginIOBoundData &info) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            if (serializer && serializer->SupportedProtocols().contains(o.protocol))
            {
                bool result = serializer->SetOutboundInfo(o.protocol, o.outboundSettings, info);
                if (result)
                    return result;
            }
        }
        return false;
    }

#if QV2RAYBASE_FEATURE(subscriptions)
    std::optional<std::shared_ptr<SubscriptionDecoder>> PluginAPIHost::Subscription_QueryType(const QString &type) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_SUBSCRIPTION_ADAPTER))
        {
            auto adapterInterface = plugin->pinterface->SubscriptionAdapter();
            if (adapterInterface)
                for (const auto &subscriptionInfo : adapterInterface->SupportedSubscriptionTypes())
                    if (subscriptionInfo.type == type)
                        return adapterInterface->GetSubscriptionDecoder(type);
        }
        return std::nullopt;
    }

    QList<std::pair<const PluginInfo *, SubscriptionInfoObject>> PluginAPIHost::Subscription_GetAllAdapters() const
    {
        QList<std::pair<const PluginInfo *, SubscriptionInfoObject>> list;
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_SUBSCRIPTION_ADAPTER))
        {
            auto adapterInterface = plugin->pinterface->SubscriptionAdapter();
            if (adapterInterface)
                for (const auto &subscriptionInfo : adapterInterface->SupportedSubscriptionTypes())
                    list << std::make_pair(plugin, subscriptionInfo);
        }
        return list;
    }
#endif

    std::optional<QString> PluginAPIHost::Outbound_Serialize(const QString &name, const OutboundObject &outbound) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            if (serializer && serializer->SupportedProtocols().contains(outbound.protocol))
            {
                const auto result = serializer->Serialize(name, outbound);
                if (result)
                    return result;
            }
        }
        return std::nullopt;
    }

    std::optional<std::pair<QString, OutboundObject>> PluginAPIHost::Outbound_Deserialize(const QString &link) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            for (const auto &prefix : serializer->SupportedLinkPrefixes())
            {
                if (link.startsWith(prefix))
                {
                    const auto outboundObject = serializer->Deserialize(link);
                    if (outboundObject)
                        return outboundObject;
                }
            }
        }
        return std::nullopt;
    }

    void PluginAPIHost::SendEventInternal(const ConnectionStats::EventObject &object) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(Qv2rayPlugin::COMPONENT_EVENT_HANDLER))
            plugin->pinterface->EventHandler()->ProcessEvent(object);
    }

    void PluginAPIHost::SendEventInternal(const SystemProxy::EventObject &object) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(Qv2rayPlugin::COMPONENT_EVENT_HANDLER))
            plugin->pinterface->EventHandler()->ProcessEvent(object);
    }

    void PluginAPIHost::SendEventInternal(const Connectivity::EventObject &object) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(Qv2rayPlugin::COMPONENT_EVENT_HANDLER))
            plugin->pinterface->EventHandler()->ProcessEvent(object);
    }

    void PluginAPIHost::SendEventInternal(const ConnectionEntry::EventObject &object) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(Qv2rayPlugin::COMPONENT_EVENT_HANDLER))
            plugin->pinterface->EventHandler()->ProcessEvent(object);
    }
} // namespace Qv2rayBase::Plugin
