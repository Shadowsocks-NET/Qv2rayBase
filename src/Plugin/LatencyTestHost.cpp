#include "Plugin/LatencyTestHost.hpp"

#include "src/private/LatencyTestThread.hpp"

namespace Qv2rayBase::Plugin
{
    class LatencyTestHostPrivate
    {
      public:
        // we're not introduce multi latency test thread for now,
        // cause it's easy to use a scheduler like round-robin scheme
        // and libuv event loop is fast.
        Qv2rayBase::_private::LatencyTestThread *latencyThread;
    };

    LatencyTestHost::LatencyTestHost(QObject *parent) : QObject(parent)
    {
        d_ptr.reset(new LatencyTestHostPrivate);
        Q_D(LatencyTestHost);
        d->latencyThread = new Qv2rayBase::_private::LatencyTestThread(this);
        d->latencyThread->start();
    }

    LatencyTestHost::~LatencyTestHost()
    {
        Q_D(LatencyTestHost);
        d->latencyThread->stopLatencyTest();
        d->latencyThread->wait();
    }

    void LatencyTestHost::TestLatency(const ConnectionId &connectionId, const LatencyTestEngineId &engine)
    {
        Q_D(LatencyTestHost);
        d->latencyThread->pushRequest(connectionId, engine);
    }

    void LatencyTestHost::StopAllLatencyTest()
    {
        Q_D(LatencyTestHost);
        d->latencyThread->stopLatencyTest();
        d->latencyThread->wait();
        d->latencyThread->start();
    }

} // namespace Qv2rayBase::Plugin
