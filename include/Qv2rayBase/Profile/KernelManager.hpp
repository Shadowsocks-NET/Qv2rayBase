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
#include "Common/ProfileHelpers.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Qv2rayBaseLibrary.hpp"

namespace Qv2rayBase::Profile
{
    class KernelManagerPrivate;
    class QV2RAYBASE_EXPORT KernelManager : public QObject
    {
        Q_OBJECT
      public:
        explicit KernelManager(QObject *parent = nullptr);
        ~KernelManager();

        std::optional<QString> StartConnection(const ConnectionGroupPair &id, const ProfileContent &root);
        void StopConnection();
        const ConnectionGroupPair CurrentConnection() const;
        size_t ActiveKernelCount() const;
        const QMap<QString, PluginIOBoundData> GetCurrentConnectionInboundInfo() const;

      signals:
        void OnConnected(const ConnectionGroupPair &id);
        void OnDisconnected(const ConnectionGroupPair &id);
        void OnCrashed(const ConnectionGroupPair &id, const QString &errMessage);
        void OnKernelLogAvailable(const ConnectionGroupPair &id, const QString &log);

#if QV2RAYBASE_FEATURE(statistics)
      signals:
        void OnStatsDataAvailable(const ConnectionGroupPair &id, StatisticsObject::StatisticsType type, quint64 upspeed, quint64 downspeed);
      private slots:
        void OnPluginStatsDataRcvd_p(const quint64 uploadSpeed, const quint64 downloadSpeed);
#endif

      private slots:
        void OnKernelCrashed_p(const QString &msg);
        void OnPluginKernelLog_p(const QString &log);
        void OnV2RayKernelLog_p(const QString &log);

      private:
        void emitLogMessage(const QString &);

      private:
        Q_DECLARE_PRIVATE(KernelManager)
    };
} // namespace Qv2rayBase::Profile
