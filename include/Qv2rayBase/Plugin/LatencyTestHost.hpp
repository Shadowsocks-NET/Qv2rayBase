#pragma once

#include "Handlers/LatencyTestHandler.hpp"

#include <Qv2rayBase_export.h>

namespace Qv2rayBase::Plugin
{
    class LatencyTestHostPrivate;
    class QV2RAYBASE_EXPORT LatencyTestHost : public QObject
    {
        Q_OBJECT
      public:
        explicit LatencyTestHost(QObject *parent = nullptr);
        virtual ~LatencyTestHost();
        void TestLatency(const ConnectionId &connectionId, const LatencyTestEngineId &id);
        void StopAllLatencyTest();

      signals:
        void OnLatencyTestCompleted(const ConnectionId &id, const Qv2rayPlugin::Latency::LatencyTestResponse &data);

      private:
        QScopedPointer<LatencyTestHostPrivate> d_ptr;
        Q_DECLARE_PRIVATE(LatencyTestHost)
    };
} // namespace Qv2rayBase::Plugin
