#pragma once
#include "Handlers/LatencyTestHandler.hpp"

#include <QThread>

namespace uvw
{
    class TimerHandle;
}

namespace Qv2rayBase::Plugin
{
    class LatencyTestHost;
    class LatencyTestThread : public QThread
    {
        Q_OBJECT
      public:
        explicit LatencyTestThread(QObject *parent = nullptr);
        void stopLatencyTest();
        void pushRequest(const ConnectionId &id, const LatencyTestEngineId &engine);

      protected:
        void run() override;

      private:
        void doTest(Qv2rayBase::Plugin::LatencyTestHost *parent, uvw::TimerHandle &handle);

      private:
        std::shared_ptr<uvw::Loop> loop;
        bool isStop = false;
        std::shared_ptr<uvw::TimerHandle> stopTimer;
        std::vector<Qv2rayPlugin::Latency::LatencyTestRequest> requests;
        std::mutex m;
    };
} // namespace Qv2rayBase::_private
