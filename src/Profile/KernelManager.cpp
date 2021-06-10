#include "Profile/KernelManager.hpp"

//#include "components/port/QvPortDetector.hpp"
#include "Common/Utils.hpp"
#include "Handlers/KernelHandler.hpp"
#include "Models/Settings.hpp"
#include "Profile/Generator.hpp"
//#include "core/kernel/V2RayKernelInteractions.hpp"

#include <QJsonIO.hpp>

#define QV_MODULE_NAME "KernelHandler"

namespace Qv2rayBase::Profile
{
    using namespace Qv2rayPlugin::Kernel;
    using namespace Qv2rayPlugin::Outbound;
    using namespace Qv2rayPlugin::Event;
    class KernelManagerPrivate
    {
      public:
        qsizetype pluginLogPrefixPadding = 0;
        QMap<QString, int> GetInboundProtocolPorts() const
        {
            QMap<QString, int> result;
            for (const auto &[tag, info] : inboundInfo.toStdMap())
            {
                result[info[IOBOUND_DATA_TYPE::IO_PROTOCOL].toString()] = info[IOBOUND_DATA_TYPE::IO_PORT].toInt();
            }
            return result;
        }

        // Since QMap does not support std::unique_ptr, we use std::map<>
        std::list<std::pair<QString, std::unique_ptr<PluginKernel>>> activeKernels;
        QMap<QString, PluginIOBoundData> inboundInfo;
        //        Qv2rayBase::core::kernel::V2RayKernelInstance *vCoreInstance = nullptr;
        ConnectionGroupPair currentId = {};
    };

#define isConnected (!d->activeKernels.empty())
    KernelManager::KernelManager(QObject *parent) : QObject(parent)
    {
        //        vCoreInstance = new V2RayKernelInstance(this);
        //        connect(vCoreInstance, &V2RayKernelInstance::OnNewStatsDataArrived, this, &KernelManager::OnV2RayStatsDataRcvd_p);
        //        connect(vCoreInstance, &V2RayKernelInstance::OnProcessOutputReadyRead, this, &KernelManager::OnV2RayKernelLog_p);
        //        connect(vCoreInstance, &V2RayKernelInstance::OnProcessErrored, this, &KernelManager::OnKernelCrashed_p);
    }

    size_t KernelManager::ActivePluginKernelsCount() const
    {
        Q_D(const KernelManager);
        return d->activeKernels.size();
    }

    const QMap<QString, PluginIOBoundData> KernelManager::GetCurrentConnectionInboundInfo() const
    {
        Q_D(const KernelManager);
        return d->inboundInfo;
    }

    const QStringList KernelManager::GetActiveKernelProtocols() const
    {
        Q_D(const KernelManager);
        QStringList list;
        for (const auto &[protocol, kernel] : d->activeKernels)
        {
            list << protocol;
        }
        return list;
    }

    const ConnectionGroupPair KernelManager::CurrentConnection() const
    {
        Q_D(const KernelManager);
        return d->currentId;
    }

    KernelManager::~KernelManager()
    {
        StopConnection();
    }

    //    std::optional<QString> KernelInstanceHandler::CheckPort(const QMap<QString, PluginIOBoundData> &info, int plugins)
    //    {
    //        QStringList portDetectionErrorMessage;
    //        auto portDetectionMsg = tr("There are other processes occupying the ports necessary to start the connection:") + NEWLINE + NEWLINE;
    //        for (const auto &[key, value] : info.toStdMap())
    //        {
    //#pragma message("TODO")
    //            //            const auto address = value[IOBOUND::ADDRESS].toString();
    //            //            const auto port = value[IOBOUND::PORT].toInt();
    //            //            const auto result = components::port::CheckTCPPortStatus(address, port);
    //            //            if (!result)
    //            //                portDetectionErrorMessage << tr("Endpoint: %1:%2 for inbound: \"%3\"").arg(address).arg(port).arg(key);
    //        }
    //        if (GlobalConfig.pluginConfig->v2rayIntegration)
    //        {
    //            for (auto i = 0; i < plugins; i++)
    //            {
    //#pragma message("TODO")
    //                //                const auto thisPort = GlobalConfig.pluginConfig->portAllocationStart + i;
    //                //                const auto result = components::port::CheckTCPPortStatus("127.0.0.1", thisPort);
    //                //                if (!result)
    //                //                    portDetectionErrorMessage << tr("Local port: %1 for plugin integration.").arg(thisPort);
    //            }
    //        }
    //        if (!portDetectionErrorMessage.isEmpty())
    //        {
    //            portDetectionMsg += portDetectionErrorMessage.join(NEWLINE);
    //            return portDetectionMsg;
    //        }
    //        else
    //        {
    //            return std::nullopt;
    //        }
    //    }

    std::optional<QString> KernelManager::StartConnection(const ConnectionGroupPair &id, ProfileContent fullConfig)
    {
        Q_D(KernelManager);
        StopConnection();
        d->inboundInfo = GetInboundsInfo(fullConfig);
        //
        const auto inboundPorts = d->GetInboundProtocolPorts();
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ GetDisplayName(id.connectionId), inboundPorts, Connectivity::Connecting });
        // QList<std::tuple<QString, int, QString>> inboundInfo;
        // for (const auto &inbound_v : fullConfig["inbounds"].toArray())
        //{
        //    const auto &inbound = inbound_v.toObject();
        //    inboundInfo.push_back({ inbound["protocol"].toString(), inbound["port"].toInt(), inbound["tag"].toString() });
        //}
        //
        //        if (GlobalConfig.pluginConfig->v2rayIntegration)
        //        {
        // Process outbounds.
        OUTBOUNDS processedOutbounds;
        auto pluginPort = Qv2rayBaseLibrary::GetConfig()->plugin_port_allocation;
        //
        for (auto i = 0; i < fullConfig["outbounds"].toArray().count(); i++)
        {
            auto outbound = QJsonIO::GetValue(fullConfig, "outbounds", i).toObject();
            const auto outProtocol = outbound["protocol"].toString();
            const auto kid = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_QueryProtocol(outProtocol);
            if (kid.isNull())
            {
                // Normal outbound, or the one without a plugin supported.
                // Marked as processed.
                processedOutbounds.push_back(outbound);
                LOG("Outbound protocol " + outProtocol + " is not a registered plugin outbound.");
                continue;
            }
            LOG("Creating kernel plugin instance for protocol" + outProtocol);
            auto kernel = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_Create(kid);
            // New object does not need disconnect?
            // disconnect(kernel, &QvPluginKernel::OnKernelStatsAvailable, this, &KernelInstanceHandler::OnStatsDataArrived_p);
            //
            QMap<KernelOptionFlags, QVariant> _inboundSettings;

            LOG("V2RayIntegration: " + QSTRN(pluginPort) + " = " + outProtocol);
            _inboundSettings[KERNEL_HTTP_ENABLED] = false;
            _inboundSettings[KERNEL_SOCKS_ENABLED] = true;
            _inboundSettings[KERNEL_SOCKS_PORT] = pluginPort;
#pragma message("TODO")
            //            _inboundSettings[KERNEL_SOCKS_UDP_ENABLED] = *GlobalConfig.inboundConfig->socksSettings->enableUDP;
            //            _inboundSettings[KERNEL_SOCKS_LOCAL_ADDRESS] = *GlobalConfig.inboundConfig->socksSettings->localIP;
            _inboundSettings[KERNEL_LISTEN_ADDRESS] = "127.0.0.1";
            LOG("Sending connection settings to kernel.");
            kernel->SetConnectionSettings(_inboundSettings, outbound["settings"].toObject());
            d->activeKernels.push_back({ outProtocol, std::move(kernel) });
            //
            const auto pluginOutSettings = QJsonObject{ { "address", "127.0.0.1" }, { "port", pluginPort } };
            const auto pluginOut = QJsonObject{ { "tag", outbound["tag"] }, { "protocol", "socks" }, { "settings", pluginOutSettings } };
            //
            // Add the integration outbound to the list.
            processedOutbounds.push_back(pluginOut);
            pluginPort++;
        }
        LOG("Applying new outbound settings.");
        fullConfig["outbounds"] = processedOutbounds;
#pragma message("TODO")
        //        RemoveEmptyMuxFilter(fullConfig);
        //        }

        //
        // ======================================================================= Start Kernels
        //
        {
            auto firstOutbound = fullConfig["outbounds"].toArray().first().toObject();
            const auto firstOutboundProtocol = firstOutbound["protocol"].toString();
            if (/*GlobalConfig.pluginConfig->v2rayIntegration*/ true)
            {
                LOG("Starting kernels with V2RayIntegration.");
                bool hasAllKernelStarted = true;
                for (auto &[outboundProtocol, pKernel] : d->activeKernels)
                {
                    LOG("Starting kernel for protocol: " + outboundProtocol);
                    bool status = pKernel->Start();
                    connect(pKernel.get(), SIGNAL(OnCrashed), this, SLOT(OnKernelCrashed_p), Qt::QueuedConnection);
                    connect(pKernel.get(), SIGNAL(OnKernelLog), this, SLOT(OnPluginKernelLog_p), Qt::QueuedConnection);
                    hasAllKernelStarted = hasAllKernelStarted && status;
                    if (!status)
                    {
                        LOG("Plugin Kernel: " + outboundProtocol + " failed to start.");
                        break;
                    }
                }
                if (!hasAllKernelStarted)
                {
                    StopConnection();
                    return tr("A plugin kernel failed to start. Please check the outbound settings.");
                }
                d->currentId = id;
                //
                // Also start V2Ray-core.
#pragma message("TODO")
                // TODO
                // auto result = vCoreInstance->StartConnection(fullConfig);
                //
                //                if (/*result.has_value()*/ false)
                //                {
                //                    StopConnection();
                //                    Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ GetDisplayName(id.connectionId), inboundPorts,
                //                    Connectivity::Disconnected }); return result;
                //                }
                //                else
                {
                    emit OnConnected(id);
                    Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ GetDisplayName(id.connectionId), inboundPorts, Connectivity::Connected });
                }
            }
#pragma message("TODO")
            //            else if (const auto kid = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_QueryProtocol(firstOutboundProtocol); !kid.isNull())
            //            {
            //                // Connections without V2Ray Integration will have and ONLY have ONE kernel.
            //                LOG("Starting kernel " + firstOutboundProtocol + " without V2Ray Integration");
            //                {
            //                    auto kernel = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_Create(kid);
            //                    d->activeKernels.push_back({ firstOutboundProtocol, std::move(kernel) });
            //                }
            //                Q_ASSERT(d->activeKernels.size() == 1);
            //#define theKernel (d->activeKernels.front().second.get())
            //                connect(theKernel, SIGNAL(OnStatsAvailable), this, SIGNAL(OnPluginStatsDataRcvd_p), Qt::QueuedConnection);
            //                connect(theKernel, SIGNAL(OnCrashed), this, SIGNAL(OnKernelCrashed_p), Qt::QueuedConnection);
            //                connect(theKernel, SIGNAL(OnKernelLog), this, SIGNAL(OnPluginKernelLog_p), Qt::QueuedConnection);
            //                d->currentId = id;
            //                //
            //                QMap<KernelOptionFlags, QVariant> pluginSettings;

            //                for (const auto &v : qAsConst(d->inboundInfo))
            //                {
            //                    const auto protocol = v[IOBOUND_DATA_TYPE::IO_PROTOCOL].toString();
            //                    const auto port = v[IOBOUND_DATA_TYPE::IO_PORT].toInt();

            //                    if (protocol != "http" && protocol != "socks")
            //                        continue;
            //                    pluginSettings[KERNEL_HTTP_ENABLED] = pluginSettings[KERNEL_HTTP_ENABLED].toBool() || protocol == "http";
            //                    pluginSettings[KERNEL_SOCKS_ENABLED] = pluginSettings[KERNEL_SOCKS_ENABLED].toBool() || protocol == "socks";
            //                    pluginSettings.insert(protocol.toLower() == "http" ? KERNEL_HTTP_PORT : KERNEL_SOCKS_PORT, port);
            //                }

            //                pluginSettings[KERNEL_SOCKS_UDP_ENABLED] = *GlobalConfig.inboundConfig->socksSettings->enableUDP;
            //                pluginSettings[KERNEL_SOCKS_LOCAL_ADDRESS] = *GlobalConfig.inboundConfig->socksSettings->localIP;
            //                pluginSettings[KERNEL_LISTEN_ADDRESS] = *GlobalConfig.inboundConfig->listenip;
            //                //
            //                theKernel->SetConnectionSettings(pluginSettings, firstOutbound["settings"].toObject());
            //                bool kernelStarted = theKernel->Start();
            //#undef theKernel
            //                if (kernelStarted)
            //                {
            //                    emit OnConnected(id);
            //                    PluginHost->Event_Send<Connectivity>({ GetDisplayName(id.connectionId), inboundPorts, Connectivity::Connected });
            //                }
            //                else
            //                {
            //                    return tr("A plugin kernel failed to start. Please check the outbound settings.");
            //                    StopConnection();
            //                }
            //            }
            //            else
            //            {
            //                LOG("Starting V2Ray without plugin.");
            //                currentId = id;
            //                auto result = vCoreInstance->StartConnection(fullConfig);
            //                if (result.has_value())
            //                {
            //                    PluginHost->Event_Send<Connectivity>({ GetDisplayName(id.connectionId), inboundPorts, Connectivity::Disconnected });
            //                    StopConnection();
            //                    return result;
            //                }
            //                else
            //                {
            //                    emit OnConnected(id);
            //                    PluginHost->Event_Send<Connectivity>({ GetDisplayName(id.connectionId), inboundPorts, Connectivity::Connected });
            //                }
            //            }
        }
        // Return
        return std::nullopt;
    }

    void KernelManager::OnKernelCrashed_p(const QString &msg)
    {
        Q_D(KernelManager);
        StopConnection();
        emit OnCrashed(d->currentId, msg);
    }

    void KernelManager::emitLogMessage(const QString &l)
    {
        Q_D(KernelManager);
        emit OnKernelLogAvailable(d->currentId, l);
    }

    void KernelManager::OnPluginKernelLog_p(const QString &log)
    {
        Q_D(KernelManager);
        if (d->pluginLogPrefixPadding <= 0)
            for (const auto &[_, kernel] : d->activeKernels)
                d->pluginLogPrefixPadding = std::max(d->pluginLogPrefixPadding, Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetName(kernel->KernelId()).length());

        const auto kernel = static_cast<PluginKernel *>(sender());
        const auto name = kernel ? Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetName(kernel->KernelId()) : "UNKNOWN";
        for (const auto &line : SplitLines(log))
            emitLogMessage(QString("[%1] ").arg(name, d->pluginLogPrefixPadding) + line.trimmed());
    }

    void KernelManager::OnV2RayKernelLog_p(const QString &log)
    {
        for (const auto &line : SplitLines(log))
            emitLogMessage(line.trimmed());
    }

    void KernelManager::StopConnection()
    {
        Q_D(KernelManager);
        if (isConnected)
        {
            const auto inboundPorts = d->GetInboundProtocolPorts();
            Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ GetDisplayName(d->currentId.connectionId), inboundPorts, Connectivity::Disconnecting });

#pragma message("TODO")
            //            if (vCoreInstance->IsKernelRunning())
            //            {
            //                vCoreInstance->StopConnection();
            //            }
            for (const auto &[kernel, kernelObject] : d->activeKernels)
            {
                LOG("Stopping plugin kernel: " + kernel);
                kernelObject->Stop();
            }
            d->pluginLogPrefixPadding = 0;
            emit OnDisconnected(d->currentId);
            Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ GetDisplayName(d->currentId.connectionId), inboundPorts, Connectivity::Disconnected });
        }
        else
        {
            LOG("Cannot disconnect when there's nothing connected.");
        }
        d->currentId.clear();
        d->activeKernels.clear();
    }

#if QV2RAYBASE_FEATURE(statistics)
    void KernelManager::OnV2RayStatsDataRcvd_p(const QMap<StatisticsType, QvStatsSpeed> &data)
    {
        if (isConnected)
        {
            emit OnStatsDataAvailable(currentId, data);
        }
    }

    void KernelManager::OnPluginStatsDataRcvd_p(const long uploadSpeed, const long downloadSpeed)
    {
        OnV2RayStatsDataRcvd_p({ { API_OUTBOUND_PROXY, { uploadSpeed, downloadSpeed } } });
    }
#endif
} // namespace Qv2rayBase::Profile
