#include "Plugin/PluginAPIHost.hpp"

#include "Common/Utils.hpp"
#include "Plugin/PluginManagerCore.hpp"
#include "Qv2rayBaseLibrary.hpp"

#define QV_MODULE_NAME "PluginAPIHost"

using namespace Qv2rayPlugin;

namespace Qv2rayBase::Plugins
{
    using namespace Qv2rayPlugin::Event;
    using namespace Qv2rayPlugin::Kernel;
    using namespace Qv2rayPlugin::Outbound;
    using namespace Qv2rayPlugin::Subscription;

    class PluginAPIHostPrivate
    {
      public:
        QHash<QUuid, Qv2rayPlugin::Kernel::KernelInfo> kernels;
    };

    PluginAPIHost::PluginAPIHost()
    {
    }

    PluginAPIHost::~PluginAPIHost()
    {
    }

    //    void PluginAPIHost::LoadPlugins()
    //    {
    //        Q_D(PluginAPIHost);
    //        Qv2rayBaseLibrary::PluginManagerCore()->LoadPlugins();
    //        for (const auto &p : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_KERNEL))
    //            for (const auto &kinfo : p->pinterface->KernelInterface()->GetKernels())
    //                d->kernels.insert(kinfo.Id, kinfo);
    //    }

    std::unique_ptr<PluginKernel> PluginAPIHost::Kernel_Create(const QUuid &kid) const
    {
        Q_D(const PluginAPIHost);
        return d->kernels[kid].Create();
    }

    QString PluginAPIHost::Kernel_GetName(const QUuid &kid) const
    {
        Q_D(const PluginAPIHost);
        return d->kernels[kid].Name;
    }

    QUuid PluginAPIHost::Kernel_QueryProtocol(const QString &protocol) const
    {
        Q_D(const PluginAPIHost);
        for (const auto &k : d->kernels)
            if (k.SupportedProtocols.contains(protocol))
                return k.Id;
        return {};
    }

    std::optional<PluginIOBoundData> PluginAPIHost::Outbound_GetData(const QString &protocol, const QJsonObject &o) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            if (serializer && serializer->SupportedProtocols().contains(protocol))
            {
                auto info = serializer->GetOutboundInfo(protocol, o);
                if (info)
                    return info;
            }
        }
        return std::nullopt;
    }

    bool PluginAPIHost::Outbound_SetData(const QString &protocol, QJsonObject &o, const PluginIOBoundData &info) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            if (serializer && serializer->SupportedProtocols().contains(protocol))
            {
                bool result = serializer->SetOutboundInfo(protocol, o, info);
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

    std::optional<QString> PluginAPIHost::Outbound_Serialize(const PluginOutboundDescriptor &info) const
    {
        for (const auto &plugin : Qv2rayBaseLibrary::PluginManagerCore()->GetPlugins(COMPONENT_OUTBOUND_HANDLER))
        {
            auto serializer = plugin->pinterface->OutboundHandler();
            if (serializer && serializer->SupportedProtocols().contains(info.Protocol))
            {
                const auto result = serializer->Serialize(info);
                if (result)
                    return result;
            }
        }
        return std::nullopt;
    }

    std::optional<PluginOutboundDescriptor> PluginAPIHost::Outbound_Deserialize(const QString &link) const
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
} // namespace Qv2rayBase::Plugins
