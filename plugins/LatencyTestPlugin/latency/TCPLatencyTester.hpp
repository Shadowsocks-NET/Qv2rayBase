#pragma once

#include "DNSBase.hpp"

namespace Qv2rayBase::BuiltinPlugins::Latency
{
    class Static_TCP_LatencyTestEngine : public CommonDNSBasedAsyncLatencyTestEngine<Static_TCP_LatencyTestEngine>
    {
      public:
        virtual ~Static_TCP_LatencyTestEngine() = default;
        using CommonDNSBasedAsyncLatencyTestEngine<Static_TCP_LatencyTestEngine>::CommonDNSBasedAsyncLatencyTestEngine;

      protected:
        virtual void Prepare(std::shared_ptr<uvw::Loop>) override;
        virtual void StartTest(std::shared_ptr<uvw::Loop>) override;
    };
} // namespace Qv2rayBase::StaticPlugin
