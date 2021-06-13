#pragma once

#include "DNSBase.hpp"

namespace Qv2rayBase::BuiltinPlugins::Latency
{
    class Static_ICMP_LatencyTestEngine : public CommonDNSBasedAsyncLatencyTestEngine<Static_ICMP_LatencyTestEngine>
    {
      public:
        virtual ~Static_ICMP_LatencyTestEngine();
        using CommonDNSBasedAsyncLatencyTestEngine<Static_ICMP_LatencyTestEngine>::CommonDNSBasedAsyncLatencyTestEngine;

      protected:
        virtual void Prepare(std::shared_ptr<uvw::Loop>) override;
        virtual void StartTest(std::shared_ptr<uvw::Loop>) override;

      private:
        bool checkAndFinalize();

#ifdef Q_OS_UNIX
        // unix
        void deinit();
        // number incremented with every echo request packet send
        unsigned short seq = 1;
        // socket
        int socketId = -1;
        std::shared_ptr<uvw::TimerHandle> timeoutTimer;
        std::shared_ptr<uvw::PollHandle> pollHandle;
        std::vector<timeval> startTimevals;
#elif defined(Q_OS_WIN)
      private:
        void pingImpl();

      private:
        uint64_t timeout = 3000;
        std::shared_ptr<uvw::TimerHandle> waitHandleTimer;
#endif
    };
} // namespace Qv2rayBase::StaticPlugin
