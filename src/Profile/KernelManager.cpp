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

#include "Profile/KernelManager.hpp"

#include "Common/Settings.hpp"
#include "Common/Utils.hpp"
#include "Handlers/KernelHandler.hpp"
#include "Interfaces/IConfigurationGenerator.hpp"
#include "private/Profile/KernelManager_p.hpp"

#define QV_MODULE_NAME "KernelHandler"

namespace Qv2rayBase::Profile
{
    using namespace Qv2rayPlugin::Kernel;
    using namespace Qv2rayPlugin::Outbound;
    using namespace Qv2rayPlugin::Event;

    KernelManager::KernelManager(QObject *parent) : QObject(parent)
    {
    }

    size_t KernelManager::ActiveKernelCount() const
    {
        Q_D(const KernelManager);
        return d->kernels.size();
    }

    const QMap<QString, PluginIOBoundData> KernelManager::GetCurrentConnectionInboundInfo() const
    {
        Q_D(const KernelManager);
        return d->inboundInfo;
    }

    const ConnectionGroupPair KernelManager::CurrentConnection() const
    {
        Q_D(const KernelManager);
        return d->current;
    }

    KernelManager::~KernelManager()
    {
        StopConnection();
    }

    std::optional<QString> KernelManager::StartConnection(const ConnectionGroupPair &id, const ProfileContent &root)
    {
        auto fullProfile = root;
        Q_D(KernelManager);
        StopConnection();

        // In case of the configuration did not specify a kernel explicitly
        // find a kernel with router, and with as many protocols supported as possible.
        auto DefaultKernelID = fullProfile.defaultKernel.isNull() ? Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetDefaultKernel() : fullProfile.defaultKernel;

        const auto DefaultKernelInfo = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetInfo(DefaultKernelID);

        // Leave, nothing cound be found.
        if (DefaultKernelInfo.Name.isEmpty())
            return QObject::tr("Cannot find the specified kernel");

        QSet<QString> protocols;
        for (const auto &out : fullProfile.outbounds)
        {
            // TODO log
            protocols << out.protocol;
        }

        // Remove protocols which are already supported by the main kernel
        protocols -= DefaultKernelInfo.SupportedProtocols;

        // Process outbounds.
        QList<OutboundObject> processedOutbounds;
        auto pluginPort = Qv2rayBaseLibrary::GetConfig()->plugin_config.plugin_port_allocation;
        for (auto outbound : fullProfile.outbounds)
        {
            if (DefaultKernelInfo.SupportedProtocols.contains(outbound.protocol))
            {
                // Use the default kernel
                processedOutbounds << outbound;
                continue;
            }

            const auto kid = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_QueryProtocol(QSet{ outbound.protocol });
            if (kid.isNull())
            {
                // Expected a plugin, but found nothing
                QvLog() << "Outbound protocol" << outbound.protocol << "is not a registered plugin outbound.";
                return tr("Cannot find a kernel for outbound protocol: ") + outbound.protocol;
            }

            const auto kinfo = Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetInfo(kid);
            auto pkernel = kinfo.Create();

            {
                QMap<KernelOptionFlags, QVariant> kernelOption;
                kernelOption[KERNEL_SOCKS_ENABLED] = true;
                kernelOption[KERNEL_SOCKS_PORT] = pluginPort;
                // inboundSettings[KERNEL_SOCKS_UDP_ENABLED] = *GlobalConfig.inboundConfig->socksSettings->enableUDP;
                // inboundSettings[KERNEL_SOCKS_LOCAL_ADDRESS] = *GlobalConfig.inboundConfig->socksSettings->localIP;
                kernelOption[KERNEL_LISTEN_ADDRESS] = "127.0.0.1";
                QvLog() << "Sending connection settings to kernel.";
                pkernel->SetConnectionSettings(kernelOption, outbound.settings);
            }

            d->kernels.push_back({ outbound.protocol, std::move(pkernel) });

            const OutboundSettings pluginOutSettings(QJsonObject{ { "address", "127.0.0.1" }, { "port", pluginPort } });
            outbound.protocol = QStringLiteral("socks");
            outbound.settings = pluginOutSettings;

            // Add the integration outbound to the list.
            processedOutbounds.append(outbound);
            pluginPort++;
        }

        QvLog() << "Applying new outbound settings.";
        fullProfile.outbounds = processedOutbounds;

        bool hasAllKernelStarted = true;
        for (auto &[protocol, kernel] : d->kernels)
        {
            QvLog() << "Starting kernel for protocol:" << protocol;
            bool status = kernel->Start();
            connect(kernel.get(), SIGNAL(OnCrashed), this, SLOT(OnKernelCrashed_p), Qt::QueuedConnection);
            connect(kernel.get(), SIGNAL(OnKernelLog), this, SLOT(OnPluginKernelLog_p), Qt::QueuedConnection);
            hasAllKernelStarted &= status;
            if (!status)
            {
                QvLog() << "Plugin Kernel:" << protocol << "failed to start.";
                break;
            }
        }

        // Start the default kernel
        {
            auto defaultKernel = DefaultKernelInfo.Create();
            defaultKernel->SetProfileContent(fullProfile);
            hasAllKernelStarted &= defaultKernel->Start();
            d->kernels.push_back({ d->QV2RAYBASE_DEFAULT_KERNEL_PLACEHOLDER, std::move(defaultKernel) });
        }

        if (!hasAllKernelStarted)
        {
            StopConnection();
            return tr("A plugin kernel failed to start. Please check the outbound settings.");
        }

        // ============= Finalize =============
        // Protocol, Tag, Port

        d->inboundInfo.clear();
        for (const auto &in : fullProfile.inbounds)
            d->inboundInfo.insert(in[QStringLiteral("tag")].toString(), { { IOBOUND_DATA_TYPE::IO_PROTOCOL, in["protocol"].toString() },
                                                                          { IOBOUND_DATA_TYPE::IO_DISPLAYNAME, in["tag"].toString() },
                                                                          { IOBOUND_DATA_TYPE::IO_ADDRESS, in["listen"].toInt() } });

        d->current = id;
        emit OnConnected(id);
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ Connectivity::Connected, id, d->inboundInfo, d->outboundInfo });
        return std::nullopt;
    }

    void KernelManager::OnKernelCrashed_p(const QString &msg)
    {
        Q_D(KernelManager);
        StopConnection();
        emit OnCrashed(d->current, msg);
    }

    void KernelManager::emitLogMessage(const QString &l)
    {
        Q_D(KernelManager);
        emit OnKernelLogAvailable(d->current, l);
    }

    void KernelManager::OnPluginKernelLog_p(const QString &log)
    {
        Q_D(KernelManager);
        if (d->logPadding <= 0)
            for (const auto &[_, kernel] : d->kernels)
                d->logPadding = std::max(d->logPadding, Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetInfo(kernel->GetKernelId()).Name.length());

        const auto kernel = static_cast<PluginKernel *>(sender());
        const auto name = kernel ? Qv2rayBaseLibrary::PluginAPIHost()->Kernel_GetInfo(kernel->GetKernelId()).Name : QStringLiteral("UNKNOWN");
        for (const auto &line : SplitLines(log))
            emitLogMessage(QStringLiteral("[%1] ").arg(name, d->logPadding) + line.trimmed());
    }

    void KernelManager::OnV2RayKernelLog_p(const QString &log)
    {
        for (const auto &line : SplitLines(log))
            emitLogMessage(line.trimmed());
    }

    void KernelManager::StopConnection()
    {
        Q_D(KernelManager);
        Q_ASSERT(d->kernels.empty() == d->current.isNull());

        if (d->kernels.empty())
        {
            QvLog() << "Cannot disconnect when there's nothing connected.";
            return;
        }

        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ Connectivity::Disconnecting, d->current });

        for (const auto &[kernel, kernelObject] : d->kernels)
        {
            QvLog() << "Stopping plugin kernel:" << kernel;
            kernelObject->Stop();
        }

        d->logPadding = 0;
        Qv2rayBaseLibrary::PluginAPIHost()->Event_Send<Connectivity>({ Connectivity::Disconnected, d->current });
        emit OnDisconnected(d->current);

        d->current.clear();
        d->kernels.clear();
    }

#if QV2RAYBASE_FEATURE(statistics)
    void KernelManager::OnPluginStatsDataRcvd_p(const quint64 uploadSpeed, const quint64 downloadSpeed)
    {
        Q_D(KernelManager);
        emit OnStatsDataAvailable(d->current, StatisticsObject::API_OUTBOUND_PROXY, uploadSpeed, downloadSpeed);
    }
#endif
} // namespace Qv2rayBase::Profile
